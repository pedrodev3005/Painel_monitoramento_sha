// SubsistemaAlerta.hpp

#ifndef SUBSISTEMA_ALERTA_H
#define SUBSISTEMA_ALERTA_H

#include "NotificacaoFactory.hpp"
#include "Entidades.hpp"
#include "Logger.hpp"
#include "LimiteAlertaDAO.hpp"


/**
 * @brief Gerencia os limites de consumo e dispara notificações utilizando o Factory Method.
 */
class SubsistemaAlerta {
private:
    NotificadorFactory* notificadorFactory;
    LimiteAlertaDAO* limiteDAO; 

public:
    SubsistemaAlerta(NotificadorFactory* factory, LimiteAlertaDAO* dao) 
        : notificadorFactory(factory), limiteDAO(dao) {
        Logger::getInstance()->registrarInfo("SubsistemaAlerta", "Subsistema inicializado.");
    }

    bool definirLimite(int idUsuario, const std::string& idSHA, double limiteVolumeM3);
    double buscarLimite(int idUsuario, const std::string& idSHA);
    void notificarAlerta(int idUsuario, const AlertaConsumo& alerta, CanalAlerta canal);
    bool verificarLimiteExcedido(int idUsuario, const std::string& idSHA, double volumeAtual);
};

#endif // SUBSISTEMA_ALERTA_H