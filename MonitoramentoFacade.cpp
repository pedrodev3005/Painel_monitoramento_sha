// MonitoramentoFacade.cpp

#include "MonitoramentoFacade.hpp"
#include <iostream>
#include <chrono>

MonitoramentoFacade::MonitoramentoFacade(SubsistemaUsuarios* u, SubsistemaAlerta* a, SubsistemaDados* d)
    : subsistemaUsuarios(u), subsistemaAlerta(a), subsistemaDados(d) {
    Logger::getInstance()->registrarInfo("MonitoramentoFacade", "Fachada inicializada. Arquitetura pronta.");
    // ✅ NÃO inicia thread aqui. Só inicia quando o usuário mandar no menu.
}

MonitoramentoFacade::~MonitoramentoFacade() {
    encerrarando:
    encerrando.store(true);
    autoAtivo.store(false);
    cvAuto.notify_all();

    if (thAuto.joinable()) thAuto.join();
}

// =======================================================
// RF 1
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
// RF 2 (manual)
// =======================================================

void MonitoramentoFacade::processarLeituraDiaria(const std::string& idSHA, int idUsuario) {
    Logger::getInstance()->registrarEventoCritico("MonitoramentoFacade",
        "INICIANDO PROCESSO COMPLETO DE SHA: " + idSHA + " <-- ACAO DE AUDITORIA");

    subsistemaDados->iniciarProcessamento(idSHA, idUsuario);
}

ConsumoDTO MonitoramentoFacade::monitorarConsumoUsuario(int idUsuario, std::time_t inicio, std::time_t fim) {
    Logger::getInstance()->registrarInfo("MonitoramentoFacade", "Solicitando consolidacao de consumo (RF 2.3).");
    return subsistemaDados->monitorarConsumoUsuario(idUsuario, inicio, fim);
}

// =======================================================
// RF 3
// =======================================================

bool MonitoramentoFacade::definirLimiteAlerta(int idUsuario, const std::string& idSHA, double limiteVolumeM3) {
    return subsistemaAlerta->definirLimite(idUsuario, idSHA, limiteVolumeM3);
}

// =======================================================
// AUTO
// =======================================================

void MonitoramentoFacade::ajustarIntervaloMonitoramento(int segundos) {
    if (segundos < 1) segundos = 1;
    intervaloSeg.store(segundos);
    Logger::getInstance()->registrarInfo("MonitoramentoFacade",
        "Intervalo do monitoramento automatico ajustado para " + std::to_string(segundos) + "s.");
    cvAuto.notify_all();
}

void MonitoramentoFacade::iniciarMonitoramentoAutomaticoSHA(const std::string& idSHA, int idUsuario) {
    {
        std::lock_guard<std::mutex> lk(mtxAuto);

        // ✅ monitora só o SHA escolhido (você pode trocar pra "push_back" se quiser vários)
        shasAuto.clear();
        shasAuto.push_back({idSHA, idUsuario});
    }

    autoAtivo.store(true);
    cvAuto.notify_all();

    // se a thread ainda não existe, cria agora
    if (!thAuto.joinable()) {
        thAuto = std::thread(&MonitoramentoFacade::loopMonitoramentoAutomatico, this);
        Logger::getInstance()->registrarInfo("AUTO", "Thread de monitoramento automatico criada (aguardando comando).");
    }

    Logger::getInstance()->registrarInfo("AUTO",
        "Monitoramento automatico ATIVADO para SHA=" + idSHA + " (Usuario=" + std::to_string(idUsuario) + ").");
}

void MonitoramentoFacade::pararMonitoramentoAutomatico() {
    autoAtivo.store(false);
    cvAuto.notify_all();
    Logger::getInstance()->registrarInfo("AUTO", "Monitoramento automatico DESLIGADO.");
}

void MonitoramentoFacade::loopMonitoramentoAutomatico() {
    while (!encerrando.load()) {
        // espera até ligar ou encerrar
        {
            std::unique_lock<std::mutex> lk(mtxAuto);
            cvAuto.wait(lk, [&]() {
                return encerrando.load() || autoAtivo.load();
            });
            if (encerrando.load()) break;
        }

        // roda ciclo
        if (autoAtivo.load()) {
            std::vector<std::pair<std::string, int>> lista;
            {
                std::lock_guard<std::mutex> lk(mtxAuto);
                lista = shasAuto;
            }

            // se não tem SHA cadastrado pra monitorar, só não faz nada
            if (!lista.empty()) {
                for (auto& p : lista) {
                    const std::string& idSHA = p.first;
                    int idUsuario = p.second;
                    processarLeituraDiaria(idSHA, idUsuario);
                }
            }
        }

        // dorme intervalo, mas acorda se alguém mudar estado/intervalo
        int s = intervaloSeg.load();
        std::unique_lock<std::mutex> lk(mtxAuto);
        cvAuto.wait_for(lk, std::chrono::seconds(s), [&]() {
            return encerrando.load() || !autoAtivo.load();
        });
    }
}
