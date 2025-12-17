// SubsistemaAlerta.cpp
#include "SubsistemaAlerta.hpp"

SubsistemaAlerta::SubsistemaAlerta(LimiteAlertaDAO* dao)
    : limiteDAO(dao) {
    Logger::getInstance()->registrarInfo("SubsistemaAlerta", "Subsistema de Alerta inicializado.");
}

bool SubsistemaAlerta::definirLimite(int idUsuario, const std::string& idSHA, double limiteVolumeM3) {
    // delega no DAO
    return limiteDAO->salvarLimite(idUsuario, idSHA, limiteVolumeM3);
}

void SubsistemaAlerta::verificarLimiteExcedido(int idUsuario, const std::string& idSHA, double volumeLidoM3) {
    double limite = limiteDAO->buscarLimite(idUsuario, idSHA);

    Logger::getInstance()->registrarInfo("SubsistemaAlerta",
        "Verificando limite para SHA " + idSHA +
        " | volume=" + std::to_string(volumeLidoM3) +
        " | limite=" + std::to_string(limite)
    );

    if (limite > 0.0 && volumeLidoM3 > limite) {
        notificarLimiteExcedido(idUsuario, idSHA, volumeLidoM3, limite);
    }
}

void SubsistemaAlerta::notificarLimiteExcedido(int idUsuario, const std::string& idSHA, double volumeLidoM3, double limiteM3) {
    // Observer (Painel)
    for (auto* obs : observadores) {
        if (obs) obs->notificarLimiteExcedido(idUsuario, idSHA, volumeLidoM3, limiteM3);
    }

    // log padrão do sistema (como vc já tem no terminal)
    Logger::getInstance()->registrarInfo("SubsistemaAlerta",
        "LIMITE EXCEDIDO! SHA=" + idSHA +
        " volume=" + std::to_string(volumeLidoM3) +
        " > limite=" + std::to_string(limiteM3)
    );
}

void SubsistemaAlerta::adicionarObservador(IObservadorAlerta* obs) {
    if (!obs) return;
    if (std::find(observadores.begin(), observadores.end(), obs) == observadores.end()) {
        observadores.push_back(obs);
    }
}

void SubsistemaAlerta::removerObservador(IObservadorAlerta* obs) {
    observadores.erase(std::remove(observadores.begin(), observadores.end(), obs), observadores.end());
}
