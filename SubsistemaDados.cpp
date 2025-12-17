#include "SubsistemaDados.hpp"
#include "TemplateMonitoramento.hpp"
#include "Logger.hpp"
#include "EstrategiasOCR.hpp"

#include <memory>
#include <filesystem>
#include <system_error>
#include <chrono>

// =========================================================================
// Construtor
// =========================================================================
SubsistemaDados::SubsistemaDados(
    LeitorImagemSHA* leitor,
    SubsistemaAlerta* alerta,
    ConsumoHistoricoDAO* dao,
    SHAConfigDAO* configDao
)
    : leitorSHA(leitor),
      subsistemaAlerta(alerta),
      historicoDAO(dao),
      configDAO(configDao)
{
    Logger::getInstance()->registrarInfo("SubsistemaDados",
        "Subsistema de Dados (Facade interno) inicializado.");
}

// =========================================================================
// Helper: assinatura do arquivo (path + size + last_write_time)
// Serve pra detectar "imagem nova" mesmo se o nome repetir.
// =========================================================================
std::string SubsistemaDados::assinaturaArquivo(const std::string& caminho) {
    namespace fs = std::filesystem;

    if (caminho.empty()) return "";

    std::error_code ec1, ec2;
    auto sz = fs::file_size(caminho, ec1);
    auto ft = fs::last_write_time(caminho, ec2);

    long long t = 0;
    if (!ec2) {
        t = std::chrono::duration_cast<std::chrono::nanoseconds>(
                ft.time_since_epoch()
            ).count();
    }

    // se der erro, ainda retorna pelo menos o path
    return caminho + "|sz=" + std::to_string(ec1 ? -1LL : (long long)sz) +
           "|t=" + std::to_string(ec2 ? -1LL : t);
}

// =========================================================================
// Métodos de Leitura e Persistência
// =========================================================================
double SubsistemaDados::obterLeituraVolume(const std::string& idSHA) {
    Logger::getInstance()->registrarInfo("SubsistemaDados",
        "Acionando Leitor de Imagem (Adapter) para SHA: " + idSHA);

    Imagem img;
    TipoMedidor tipo = TipoMedidor::DIGITAL;

    // ✅ usa cache (se veio do iniciarProcessamentoSeNovaImagem)
    {
        std::lock_guard<std::mutex> lock(mtxDedup);

        auto itImg = cacheImagemPorSHA.find(idSHA);
        auto itTipo = cacheTipoPorSHA.find(idSHA);

        if (itImg != cacheImagemPorSHA.end() && itTipo != cacheTipoPorSHA.end()) {
            img = itImg->second;
            tipo = itTipo->second;

            cacheImagemPorSHA.erase(itImg);
            cacheTipoPorSHA.erase(itTipo);
        } else {
            // fallback: modo "manual" ou sem cache
            img = leitorSHA->obterImagem(idSHA);
            tipo = leitorSHA->determinarTipoMedidor(idSHA);
        }
    }

    // Strategy interpreta (OCR)
    std::unique_ptr<InterpretadorConsumo> ocr;
    if (tipo == TipoMedidor::DIGITAL) ocr = std::make_unique<InterpretadorDigital>();
    else                             ocr = std::make_unique<InterpretadorAnalogico>();

    return ocr->interpretar(img);
}

void SubsistemaDados::salvarConsumo(const std::string& idSHA, double volumeLido) {
    // 1) pega idUsuario via configDAO (como você já fez)
    ConfiguracaoSHA config = configDAO->buscarConfiguracao(idSHA);
    int idUsuario = config.idUsuario;

    if (idUsuario == -1) {
        Logger::getInstance()->registrarErro("SubsistemaDados",
            "Falha: Nao foi possivel encontrar o idUsuario para SHA: " + idSHA);
        return;
    }

    // 2) cria DTO pro DAO
    LeituraConsumo novaLeitura;
    novaLeitura.idSHA = idSHA;
    novaLeitura.volume = volumeLido;
    novaLeitura.timestamp = std::time(nullptr);

    // 3) salva no DAO
    if (historicoDAO->salvarLeitura(idUsuario, novaLeitura)) {
        Logger::getInstance()->registrarInfo("SubsistemaDados",
            "Leitura " + std::to_string(volumeLido) +
            " SALVA NO DB (Usuario ID: " + std::to_string(idUsuario) + ").");
    } else {
        Logger::getInstance()->registrarErro("SubsistemaDados",
            "Falha ao salvar consumo no DB.");
    }
}

// =========================================================================
// Método de Entrada - Cliente do Template Method
// =========================================================================
void SubsistemaDados::iniciarProcessamento(const std::string& idSHA, int idUsuario) {
    Logger::getInstance()->registrarInfo("SubsistemaDados",
        "Acionando Template Method para SHA: " + idSHA);

    LeituraSHAProcessador processor(this, subsistemaAlerta);
    processor.executarFluxo(idSHA, idUsuario);
}

// =========================================================================
// ✅ NOVO: só processa se tiver imagem nova
// - evita spam de leitura repetida quando o painel está rodando em loop
// =========================================================================
bool SubsistemaDados::iniciarProcessamentoSeNovaImagem(const std::string& idSHA, int idUsuario) {
    if (idSHA.empty()) return false;

    // 1) pega imagem atual (mais recente)
    Imagem img = leitorSHA->obterImagem(idSHA);
    if (img.caminhoArquivo.empty()) {
        Logger::getInstance()->registrarErro("SubsistemaDados",
            "iniciarProcessamentoSeNovaImagem: caminhoArquivo vazio para SHA " + idSHA);
        return false;
    }

    TipoMedidor tipo = leitorSHA->determinarTipoMedidor(idSHA);

    // 2) monta assinatura
    const std::string sig = assinaturaArquivo(img.caminhoArquivo);

    // 3) dedup + trava por SHA
    {
        std::lock_guard<std::mutex> lock(mtxDedup);

        // se já tem um processamento em andamento pra esse SHA, não entra
        if (processandoPorSHA[idSHA]) {
            return false;
        }

        auto it = ultimaAssinaturaPorSHA.find(idSHA);
        if (it != ultimaAssinaturaPorSHA.end() && it->second == sig) {
            // mesma imagem => não processa
            return false;
        }

        // marca como processando
        processandoPorSHA[idSHA] = true;

        // ✅ cache pra o Template Method não chamar obterImagem de novo
        cacheImagemPorSHA[idSHA] = img;
        cacheTipoPorSHA[idSHA] = tipo;
    }

    // 4) processa (Template Method)
    try {
        iniciarProcessamento(idSHA, idUsuario);
    } catch (...) {
        Logger::getInstance()->registrarErro("SubsistemaDados",
            "Excecao no iniciarProcessamentoSeNovaImagem para SHA " + idSHA);
        // libera o lock abaixo
    }

    // 5) atualiza assinatura e libera flag
    {
        std::lock_guard<std::mutex> lock(mtxDedup);
        ultimaAssinaturaPorSHA[idSHA] = sig;
        processandoPorSHA[idSHA] = false;
    }

    return true;
}

// =========================================================================
// Consulta de Histórico
// =========================================================================
ConsumoDTO SubsistemaDados::monitorarConsumoUsuario(int idUsuario, std::time_t inicio, std::time_t fim) {
    Logger::getInstance()->registrarInfo("SubsistemaDados",
        "Consulta consolidada de consumo para ID: " + std::to_string(idUsuario));

    return ConsumoDTO{150.75}; // simulado
}
