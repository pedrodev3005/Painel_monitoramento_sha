#include "Logger.hpp"

#include <sstream>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <filesystem>

// ====== filtro por thread (console) ======
static thread_local Logger::Nivel g_consoleMinNivel = Logger::Nivel::INFO;

static inline bool deveImprimirNoConsole(Logger::Nivel n) {
    return (int)n >= (int)g_consoleMinNivel;
}

// ====== Singleton ======
Logger* Logger::instancia = nullptr;

Logger* Logger::getInstance() {
    // simples e suficiente pro teu caso (um único processo)
    if (instancia == nullptr) {
        instancia = new Logger();
    }
    return instancia;
}

// ====== ctor/dtor ======
Logger::Logger()
    : caminhoLog("logs/monitoramento.log")
{
    try {
        std::filesystem::create_directories("logs");
    } catch (...) {
        // se falhar, a gente tenta abrir mesmo assim; se não abrir, só não grava em arquivo
    }

    std::lock_guard<std::mutex> lk(mtxArquivo);
    abrirArquivoSeNecessarioLocked();
}

Logger::~Logger() {
    std::lock_guard<std::mutex> lk(mtxArquivo);
    if (arquivo.is_open()) arquivo.close();
}

// ====== config ======
void Logger::setConsoleMinNivelThread(Nivel min) {
    g_consoleMinNivel = min;
}

void Logger::setArquivoLog(const std::string& caminho) {
    std::lock_guard<std::mutex> lk(mtxArquivo);

    caminhoLog = caminho;

    // cria diretório se existir no path (ex: logs/a/b.log)
    try {
        std::filesystem::path p(caminhoLog);
        if (p.has_parent_path()) {
            std::filesystem::create_directories(p.parent_path());
        }
    } catch (...) {}

    if (arquivo.is_open()) arquivo.close();
    abrirArquivoSeNecessarioLocked();
}

// ====== helpers ======
std::string Logger::obterTimestamp() const {
    std::time_t now = std::time(nullptr);
    std::tm* ltm = std::localtime(&now);

    std::stringstream ss;
    ss << std::put_time(ltm, "[%Y-%m-%d %H:%M:%S]");
    return ss.str();
}

void Logger::abrirArquivoSeNecessarioLocked() {
    if (arquivo.is_open()) return;

    arquivo.open(caminhoLog, std::ios::app);
    // se falhar, segue sem arquivo (não dá crash)
}

void Logger::salvarEmArquivo(const std::string& linha) {
    std::lock_guard<std::mutex> lk(mtxArquivo);
    abrirArquivoSeNecessarioLocked();

    if (arquivo.is_open()) {
        arquivo << linha << "\n";
        arquivo.flush(); // garante persistência em tempo real (pode remover se quiser performance)
    }
}

// ====== logs ======
void Logger::registrarInfo(const std::string& subsistema, const std::string& msg) {
    std::string linha = obterTimestamp() + " [INFO] [" + subsistema + "] " + msg;

    // ✅ sempre salva no arquivo
    salvarEmArquivo(linha);

    // ✅ console só se passar no filtro da thread
    if (deveImprimirNoConsole(Nivel::INFO)) {
        std::cout << linha << std::endl;
    }
}

void Logger::registrarErro(const std::string& subsistema, const std::string& msg) {
    std::string linha = obterTimestamp() + " [ERROR] [" + subsistema + "] " + msg;

    salvarEmArquivo(linha);

    if (deveImprimirNoConsole(Nivel::ERROR)) {
        std::cerr << linha << std::endl;
    }
}

void Logger::registrarEventoCritico(const std::string& subsistema, const std::string& msg) {
    std::string linha = obterTimestamp() + " [CRITICAL] [" + subsistema + "] " + msg + " <-- AÇÃO DE AUDITORIA";

    salvarEmArquivo(linha);

    if (deveImprimirNoConsole(Nivel::CRITICAL)) {
        std::cout << linha << std::endl;
    }
}

void Logger::registrarAlerta(const std::string& origem, const std::string& mensagem) {
    // mantém teu formato original no console, mas salva completo no arquivo também
    std::string linhaArquivo = obterTimestamp() + " [ALERTA] [" + origem + "] " + mensagem;
    salvarEmArquivo(linhaArquivo);

    if (deveImprimirNoConsole(Nivel::ALERTA)) {
        std::cout << "[ALERTA] [" << origem << "] " << mensagem << std::endl;
    }
}
