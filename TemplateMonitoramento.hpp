// TemplateMonitoramento.hpp (VERSÃO CORRIGIDA E ALINHADA À ARQUITETURA)

#ifndef TEMPLATE_MONITORAMENTO_HPP
#define TEMPLATE_MONITORAMENTO_HPP

#include "SubsistemaAlerta.hpp" 
#include "SubsistemaDados.hpp" // Adiciona a dependência do executor

// Forward declaration para evitar inclusão cruzada (SubsistemaDados precisa de nós)
class SubsistemaDados; 


// --- 1. Classe Abstrata (Define o Esqueleto) ---
class MonitoramentoTemplate {
protected:
    // O Template Method precisa do SubsistemaDados para EXECUTAR os passos de leitura e salvamento.
    SubsistemaDados* executorDados; 
    SubsistemaAlerta* executorAlerta; 

    // O Volume Lido é um valor temporário necessário para o fluxo
    double volumeLido;
    std::string idSHAAtual;
    int idUsuarioAtual;

    // --- Hooks (Etapas Variáveis) ---
    // O Template Method apenas DELEGA, não sabe como as ações são feitas.
    virtual double obterLeitura() = 0;
    virtual void persistirLeitura() = 0;
    virtual void verificarDispararAlerta() = 0;
    
public:
    virtual ~MonitoramentoTemplate() = default;

    /**
     * @brief O Template Method: Define o esqueleto fixo do algoritmo.
     */
    void executarFluxo(const std::string& idSHA, int idUsuario); 
};


// --- 2. Classe Concreta (Implementa os Hooks usando os Executores) ---
class LeituraSHAProcessador : public MonitoramentoTemplate {
public:
    /**
     * @brief Construtor que injeta o SubsistemaDados (executor) e o SubsistemaAlerta.
     */
    LeituraSHAProcessador(SubsistemaDados* dados, SubsistemaAlerta* alerta) {
        this->executorDados = dados;
        this->executorAlerta = alerta;
        Logger::getInstance()->registrarInfo("Template", "Processador SHA inicializado.");
    }

protected:
    // Hook 1: A Leitura é delegada ao SubsistemaDados (que usa o Adapter)
    double obterLeitura() override;

    // Hook 2: A Persistência é delegada ao SubsistemaDados (que usa o DAO)
    void persistirLeitura() override; 

    // Hook 3: A Verificação é delegada ao SubsistemaAlerta
    void verificarDispararAlerta() override;
};

#endif // TEMPLATE_MONITORAMENTO_HPP