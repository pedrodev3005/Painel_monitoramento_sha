// Logger.hpp

#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>
#include <string>
#include <ctime>
#include <iomanip>

/**
 * @brief Implementa o Padrão Singleton para o serviço de Log.
 * Garante uma única instância global para rastreamento de eventos no painel.
 */
class Logger {
private:
    static Logger* instancia;

    // Construtor privado: Impede a instanciação externa (Singleton)
    Logger() {}

    // Proíbe cópia e atribuição (reforça o Singleton)
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string obterTimestamp() const;

public:
    // Método estático de acesso global
    static Logger* getInstance();

    // Métodos de registro
    void registrarInfo(const std::string& subsistema, const std::string& msg);
    void registrarErro(const std::string& subsistema, const std::string& msg);
    void registrarEventoCritico(const std::string& subsistema, const std::string& msg);
};

#endif // LOGGER_HPP