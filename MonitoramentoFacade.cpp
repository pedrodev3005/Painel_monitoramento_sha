// MonitoramentoFacade.cpp

#include "MonitoramentoFacade.hpp"
#include <iostream>


MonitoramentoFacade::MonitoramentoFacade(SubsistemaUsuarios* u, SubsistemaAlerta* a, SubsistemaDados* d)
    : subsistemaUsuarios(u), subsistemaAlerta(a), subsistemaDados(d) {
    Logger::getInstance()->registrarInfo("MonitoramentoFacade", "Fachada inicializada. Arquitetura pronta.");
}

// =======================================================
// === RF 1: Gestão de Usuários (Delegação a SubsistemaUsuarios) ===
// =======================================================

bool MonitoramentoFacade::criarUsuario(const Usuario& dados) {
    Logger::getInstance()->registrarInfo("MonitoramentoFacade", "Solicitando criacao de usuario...");
    return subsistemaUsuarios->criarUsuario(dados);
}

Usuario MonitoramentoFacade::buscarUsuarioComContas(int idUsuario) {
    Logger::getInstance()->registrarInfo("MonitoramentoFacade", "Buscando usuario com contas vinculadas.");
    return subsistemaUsuarios->buscarUsuarioComContas(idUsuario);
}

// =======================================================
// === RF 2: Monitoramento (Inicia o Template Method no SubsistemaDados) ===
// =======================================================

void MonitoramentoFacade::processarLeituraDiaria(const std::string& idSHA, int idUsuario) {
    Logger::getInstance()->registrarEventoCritico("MonitoramentoFacade", "INICIANDO PROCESSO COMPLETO DE SHA: " + idSHA);
    
    // Chama o SubsistemaDados para iniciar o Template Method 
    // (Leitura -> Registro -> Verifica Limite -> Alerta)
    subsistemaDados->iniciarProcessamento(idSHA, idUsuario);
}

ConsumoDTO MonitoramentoFacade::monitorarConsumoUsuario(int idUsuario, std::time_t inicio, std::time_t fim) {
    Logger::getInstance()->registrarInfo("MonitoramentoFacade", "Solicitando consolidacao de consumo (RF 2.3).");
    
    // Delega a consulta consolidada ao SubsistemaDados
    return subsistemaDados->monitorarConsumoUsuario(idUsuario, inicio, fim);
}

// =======================================================
// === RF 3: Alerta e Configuração (Delegação a SubsistemaAlerta) ===
// =======================================================

bool MonitoramentoFacade::definirLimiteAlerta(int idUsuario, const std::string& idSHA, double limiteVolumeM3) {
    // Repassa a chamada para o SubsistemaAlerta com o NOVO parâmetro idSHA
    return subsistemaAlerta->definirLimite(idUsuario, idSHA, limiteVolumeM3);
}