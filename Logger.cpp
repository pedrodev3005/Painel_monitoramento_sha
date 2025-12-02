// Logger.cpp

#include "Logger.hpp"
#include <sstream>

// Inicialização da variável estática global
Logger* Logger::instancia = nullptr;

// Implementação do Singleton: verifica se a instância já existe
Logger* Logger::getInstance() {
    if (instancia == nullptr) {
        instancia = new Logger();
    }
    return instancia;
}

// Método auxiliar para obter a data/hora formatada
std::string Logger::obterTimestamp() const {
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);
    
    std::stringstream ss;
    ss << std::put_time(ltm, "[%Y-%m-%d %H:%M:%S]");
    return ss.str();
}

// Implementação dos registradores (seguindo o formato [DATA-HORA] [NÍVEL] [SUBSISTEMA] mensagem)
void Logger::registrarInfo(const std::string& subsistema, const std::string& msg) {
    std::cout << obterTimestamp() << " [INFO] [" << subsistema << "] " << msg << std::endl;
}

void Logger::registrarErro(const std::string& subsistema, const std::string& msg) {
    std::cerr << obterTimestamp() << " [ERROR] [" << subsistema << "] " << msg << std::endl;
}

void Logger::registrarEventoCritico(const std::string& subsistema, const std::string& msg) {
    // logs críticos para auditoria (pode ser gravado em um arquivo separado)
    std::cout << obterTimestamp() << " [CRITICAL] [" << subsistema << "] " << msg << " <-- AÇÃO DE AUDITORIA" << std::endl;
}