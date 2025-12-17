// SubsistemaDados.hpp (ATUALIZADO)

#ifndef SUBSISTEMA_DADOS_HPP
#define SUBSISTEMA_DADOS_HPP

#include <string>
#include <ctime>

#include "LeitorImagemSHA.hpp"       // Adapter (pega caminho da imagem)
#include "SubsistemaAlerta.hpp"      // Subsistema de alerta
#include "ConsumoHistoricoDAO.hpp"   // DAO de histórico
#include "SHAConfigDAO.hpp"          // DAO de configuração do SHA

// (Se ConsumoDTO estiver definido em outro header incluído no projeto, ok.)
struct ConsumoDTO;

class SubsistemaDados {
private:
    LeitorImagemSHA* leitorSHA;
    SubsistemaAlerta* subsistemaAlerta;
    ConsumoHistoricoDAO* historicoDAO;
    SHAConfigDAO* configDAO;

public:
    SubsistemaDados(
        LeitorImagemSHA* leitor,
        SubsistemaAlerta* alerta,
        ConsumoHistoricoDAO* dao,
        SHAConfigDAO* configDao
    );

    // Template Method (cliente)
    void iniciarProcessamento(const std::string& idSHA, int idUsuario);

    // Consulta consolidada (DAO)
    ConsumoDTO monitorarConsumoUsuario(int idUsuario, std::time_t inicio, std::time_t fim);

    // Utilitários chamados pelo Template Method
    double obterLeituraVolume(const std::string& idSHA);   // agora faz: Adapter -> Strategy OCR
    void salvarConsumo(const std::string& idSHA, double volumeLido);
};

#endif // SUBSISTEMA_DADOS_HPP
