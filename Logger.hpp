#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>
#include <fstream>
#include <mutex>

class Logger {
public:
    enum class Nivel : int {
        INFO = 0,
        ERROR = 1,
        CRITICAL = 2,
        ALERTA = 3
    };

    static Logger* getInstance();

    // ✅ controla o que aparece no console POR THREAD (o arquivo continua salvando tudo)
    void setConsoleMinNivelThread(Nivel min);

    // (opcional) troca o caminho do arquivo de log em runtime
    void setArquivoLog(const std::string& caminho);

    void registrarInfo(const std::string& subsistema, const std::string& msg);
    void registrarErro(const std::string& subsistema, const std::string& msg);
    void registrarEventoCritico(const std::string& subsistema, const std::string& msg);
    void registrarAlerta(const std::string& origem, const std::string& mensagem);

private:
    Logger();
    ~Logger();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    std::string obterTimestamp() const;
    void salvarEmArquivo(const std::string& linha);
    void abrirArquivoSeNecessarioLocked(); // chamado com mutex já travado

private:
    std::mutex mtxArquivo;
    std::ofstream arquivo;
    std::string caminhoLog;

    static Logger* instancia;
};

#endif // LOGGER_HPP
