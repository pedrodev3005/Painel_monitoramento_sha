// MonitoramentoFacade.hpp

#ifndef MONITORAMENTO_FACADE_HPP
#define MONITORAMENTO_FACADE_HPP

#include "SubsistemaUsuarios.hpp"
#include "SubsistemaAlerta.hpp"
#include "SubsistemaDados.hpp"
#include "Logger.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>
#include <vector>
#include <string>

class MonitoramentoFacade {
private:
    SubsistemaUsuarios* subsistemaUsuarios;
    SubsistemaAlerta* subsistemaAlerta;
    SubsistemaDados* subsistemaDados;

    // ===== Monitoramento automático =====
    std::atomic<bool> autoAtivo{false};
    std::atomic<bool> encerrando{false};
    std::atomic<int> intervaloSeg{5};

    std::thread thAuto;
    std::mutex mtxAuto;
    std::condition_variable cvAuto;

    // lista de SHAs monitorados automaticamente: (idSHA, idUsuario)
    std::vector<std::pair<std::string, int>> shasAuto;

    void loopMonitoramentoAutomatico();

public:
    MonitoramentoFacade(SubsistemaUsuarios* u, SubsistemaAlerta* a, SubsistemaDados* d);
    ~MonitoramentoFacade();

    // RF1
    bool criarUsuario(const Usuario& dados);
    Usuario buscarUsuarioComContas(int idUsuario);

    // RF2 (manual / “processar leitura”)
    void processarLeituraDiaria(const std::string& idSHA, int idUsuario);
    ConsumoDTO monitorarConsumoUsuario(int idUsuario, std::time_t inicio, std::time_t fim);

    // RF3
    bool definirLimiteAlerta(int idUsuario, const std::string& idSHA, double limiteVolumeM3);

    // ===== AUTO (manual start/stop) =====
    bool estaMonitorandoAutomatico() const { return autoAtivo.load(); }

    // inicia monitoramento automático de UM SHA (já cadastrado)
    void iniciarMonitoramentoAutomaticoSHA(const std::string& idSHA, int idUsuario);

    // para tudo
    void pararMonitoramentoAutomatico();

    // muda intervalo
    void ajustarIntervaloMonitoramento(int segundos);
};

#endif
