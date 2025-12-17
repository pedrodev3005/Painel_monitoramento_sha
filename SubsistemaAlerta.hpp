// SubsistemaAlerta.hpp
#ifndef SUBSISTEMA_ALERTA_HPP
#define SUBSISTEMA_ALERTA_HPP

#include <string>
#include <vector>
#include <algorithm>
#include "LimiteAlertaDAO.hpp"
#include "Logger.hpp"

// ===============================
// Observer
// ===============================
class IObservadorAlerta {
public:
    virtual ~IObservadorAlerta() = default;

    // chamando quando o limite estoura
    virtual void notificarLimiteExcedido(
        int idUsuario,
        const std::string& idSHA,
        double volumeLidoM3,
        double limiteM3
    ) = 0;
};

// ===============================
// Subsistema
// ===============================
class SubsistemaAlerta {
private:
    LimiteAlertaDAO* limiteDAO;
    std::vector<IObservadorAlerta*> observadores;

public:
    explicit SubsistemaAlerta(LimiteAlertaDAO* dao);

    bool definirLimite(int idUsuario, const std::string& idSHA, double limiteVolumeM3);

    // ESTE É O MÉTODO QUE O TEMPLATE METHOD PRECISA CHAMAR
    void verificarLimiteExcedido(int idUsuario, const std::string& idSHA, double volumeLidoM3);

    // dispara para observers
    void notificarLimiteExcedido(int idUsuario, const std::string& idSHA, double volumeLidoM3, double limiteM3);

    void adicionarObservador(IObservadorAlerta* obs);
    void removerObservador(IObservadorAlerta* obs);
};

#endif
