// MonitoramentoFacade.hpp

#ifndef MONITORAMENTO_FACADE_H
#define MONITORAMENTO_FACADE_H

// Inclui os subsistemas como dependências
#include "SubsistemaUsuarios.hpp"
#include "SubsistemaAlerta.hpp"
#include "SubsistemaDados.hpp"
#include "Logger.hpp"

class MonitoramentoFacade {
private:
    // Referências privadas para os subsistemas (Composição)
    SubsistemaUsuarios* subsistemaUsuarios;
    SubsistemaAlerta* subsistemaAlerta;
    SubsistemaDados* subsistemaDados;

public:
    // Construtor: Requer a injeção de todos os subsistemas
    MonitoramentoFacade(SubsistemaUsuarios* u, SubsistemaAlerta* a, SubsistemaDados* d);

    // --- RF 1: Gestão de Usuários (Delega ao SubsistemaUsuarios) ---
    bool criarUsuario(const Usuario& dados);
    Usuario buscarUsuarioComContas(int idUsuario);

    // --- RF 2: Monitoramento (Inicia o Template Method) ---
    void processarLeituraDiaria(const std::string& idSHA, int idUsuario);
    ConsumoDTO monitorarConsumoUsuario(int idUsuario, std::time_t inicio, std::time_t fim);

    // --- RF 3: Alerta e Configuração (Delega ao SubsistemaAlerta) ---
    bool definirLimiteAlerta(int idUsuario, const std::string& idSHA, double limiteVolumeM3);
};

#endif // MONITORAMENTO_FACADE_H