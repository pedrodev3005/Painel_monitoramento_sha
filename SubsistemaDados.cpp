// SubsistemaDados.cpp (CORRIGIDO)

#include "SubsistemaDados.hpp"
#include "TemplateMonitoramento.hpp" // Inclui a estrutura do Template Method
#include "Logger.hpp"
#include <ctime>
#include "EstrategiasOCR.hpp"
#include <memory>


// =========================================================================
// Construtor CORRIGIDO
// Recebe 4 dependências e inicializa configDAO
// =========================================================================

SubsistemaDados::SubsistemaDados(
    LeitorImagemSHA* leitor, 
    SubsistemaAlerta* alerta, 
    ConsumoHistoricoDAO* dao,
    SHAConfigDAO* configDao // <-- NOVO ARGUMENTO
)
    : leitorSHA(leitor), subsistemaAlerta(alerta), historicoDAO(dao), configDAO(configDao) // <-- Inicialização do configDAO
{
    Logger::getInstance()->registrarInfo("SubsistemaDados", "Subsistema de Dados (Facade interno) inicializado.");
}

// =========================================================================
// Métodos de Leitura e Persistência
// =========================================================================

double SubsistemaDados::obterLeituraVolume(const std::string& idSHA) {
    Logger::getInstance()->registrarInfo("SubsistemaDados", "Acionando Leitor de Imagem (Adapter) para SHA: " + idSHA);

    // 1) Adapter só pega a imagem/metadados
    Imagem img = leitorSHA->obterImagem(idSHA);
    TipoMedidor tipo = leitorSHA->determinarTipoMedidor(idSHA);

    // 2) Strategy interpreta (OCR)
    std::unique_ptr<InterpretadorConsumo> ocr;
    if (tipo == TipoMedidor::DIGITAL) ocr = std::make_unique<InterpretadorDigital>();
    else                             ocr = std::make_unique<InterpretadorAnalogico>();

    return ocr->interpretar(img);
}

void SubsistemaDados::salvarConsumo(const std::string& idSHA, double volumeLido) {

// 1. OBTEM O ID DO USUÁRIO através do SHAConfigDAO (Agora configDAO está inicializado)
ConfiguracaoSHA config = configDAO->buscarConfiguracao(idSHA);
int idUsuario = config.idUsuario; 

if (idUsuario == -1) {
Logger::getInstance()->registrarErro("SubsistemaDados", "Falha: Nao foi possivel encontrar o idUsuario para SHA: " + idSHA);
return;
}

// 2. CRIA o DTO que o DAO espera
LeituraConsumo novaLeitura;
novaLeitura.idSHA = idSHA;
novaLeitura.volume = volumeLido;
novaLeitura.timestamp = std::time(nullptr); 

// 3. CHAMA o DAO
if (historicoDAO->salvarLeitura(idUsuario, novaLeitura)) { 
Logger::getInstance()->registrarInfo("SubsistemaDados", "Leitura " + std::to_string(volumeLido) + " SALVA NO DB (Usuario ID: " + std::to_string(idUsuario) + ").");
} else {
 Logger::getInstance()->registrarErro("SubsistemaDados", "Falha ao salvar consumo no DB.");
}
}


// =========================================================================
// Método de Entrada  - Cliente do Template Method
// =========================================================================

void SubsistemaDados::iniciarProcessamento(const std::string& idSHA, int idUsuario) {
    
    Logger::getInstance()->registrarInfo("SubsistemaDados", "Acionando Template Method para SHA: " + idSHA);
    
    // 1. Cria a instância concreta do Template Method (Processador)
    // Construtor corrigido para passar SubsistemaDados e SubsistemaAlerta
    LeituraSHAProcessador processor(this, subsistemaAlerta); 
    
    // 2. Executa o fluxo fixo (Chama o método Template)
    processor.executarFluxo(idSHA, idUsuario); // <-- CORRIGIDO: Chamando o nome correto
}

// =========================================================================
// Consulta de Histórico 
// =========================================================================

ConsumoDTO SubsistemaDados::monitorarConsumoUsuario(int idUsuario, std::time_t inicio, std::time_t fim) {
Logger::getInstance()->registrarInfo("SubsistemaDados", "Consulta consolidada de consumo para ID: " + std::to_string(idUsuario));
return ConsumoDTO{150.75}; // Retorno simulado
}