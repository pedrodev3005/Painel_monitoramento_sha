// SubsistemaDados.h (CORRIGIDO)

#ifndef SUBSISTEMA_DADOS_H
#define SUBSISTEMA_DADOS_H

#include "LeitorImagemSHA.hpp"  // Adapter
#include "SubsistemaAlerta.hpp"  // Subsistema para Alerta
#include "ConsumoHistoricoDAO.hpp" // DAO para Persistência
#include "SHAConfigDAO.hpp"        // **NOVO: Inclusão obrigatória para o membro configDAO**

// Forward declarations de classes de fluxo
class LeituraSHAProcessador; 
struct ConsumoDTO; // Struct para retorno consolidado

class SubsistemaDados {
private:

    LeitorImagemSHA* leitorSHA;
    SubsistemaAlerta* subsistemaAlerta; 
    ConsumoHistoricoDAO* historicoDAO;
    SHAConfigDAO* configDAO; // Membro adicionado

public:

    SubsistemaDados(LeitorImagemSHA* leitor, SubsistemaAlerta* alerta, ConsumoHistoricoDAO* dao, SHAConfigDAO* configDao); 

    void iniciarProcessamento(const std::string& idSHA, int idUsuario);


    ConsumoDTO monitorarConsumoUsuario(int idUsuario, std::time_t inicio, std::time_t fim);

    // Métodos utilitários que o Template Method chama
    double obterLeituraVolume(const std::string& idSHA);
    void salvarConsumo(const std::string& idSHA, double volumeLido);
};

#endif // SUBSISTEMA_DADOS_H