#ifndef SUBSISTEMA_DADOS_HPP
#define SUBSISTEMA_DADOS_HPP

#include "LeitorImagemSHA.hpp"
#include "SubsistemaAlerta.hpp"
#include "ConsumoHistoricoDAO.hpp"
#include "SHAConfigDAO.hpp"

#include <string>
#include <ctime>

#include <unordered_map>
#include <mutex>

class SubsistemaDados {
public:
    SubsistemaDados(
        LeitorImagemSHA* leitor,
        SubsistemaAlerta* alerta,
        ConsumoHistoricoDAO* dao,
        SHAConfigDAO* configDao
    );

    // ====== Usados pelo Template Method ======
    double obterLeituraVolume(const std::string& idSHA);
    void salvarConsumo(const std::string& idSHA, double volumeLido);

    // ====== Entrada do Template Method ======
    void iniciarProcessamento(const std::string& idSHA, int idUsuario);

    // ✅ NOVO: só processa se aparecer imagem nova
    // Retorna true se processou, false se não tinha imagem nova (ou se estava processando)
    bool iniciarProcessamentoSeNovaImagem(const std::string& idSHA, int idUsuario);

    // ====== Consulta ======
    ConsumoDTO monitorarConsumoUsuario(int idUsuario, std::time_t inicio, std::time_t fim);

private:
    LeitorImagemSHA* leitorSHA = nullptr;
    SubsistemaAlerta* subsistemaAlerta = nullptr;
    ConsumoHistoricoDAO* historicoDAO = nullptr;
    SHAConfigDAO* configDAO = nullptr;

    // ====== Dedup / concorrência (multi-SHA) ======
    std::mutex mtxDedup;

    // assinatura da última imagem processada por SHA
    std::unordered_map<std::string, std::string> ultimaAssinaturaPorSHA;

    // evita processar o mesmo SHA simultaneamente em 2 threads
    std::unordered_map<std::string, bool> processandoPorSHA;

    // cache pra não chamar obterImagem() duas vezes no mesmo fluxo
    std::unordered_map<std::string, Imagem> cacheImagemPorSHA;
    std::unordered_map<std::string, TipoMedidor> cacheTipoPorSHA;

private:
    static std::string assinaturaArquivo(const std::string& caminho);
};

#endif // SUBSISTEMA_DADOS_HPP
