// EM TemplateMonitoramento.cpp (Versão Alinhada à Arquitetura)

#include "TemplateMonitoramento.hpp"
#include "SubsistemaDados.hpp" // Necessário para acessar métodos públicos do SubsistemaDados
#include "Logger.hpp"
#include <stdexcept>
#include <string>

// =======================================================
// 1. Template Method (Função Fixa na Classe Base)
// =======================================================

/**
 * @brief O Template Method: Define o esqueleto fixo do algoritmo.
 */
void MonitoramentoTemplate::executarFluxo(const std::string& idSHA, int idUsuario) {
    
    idSHAAtual = idSHA;
    idUsuarioAtual = idUsuario;

    Logger::getInstance()->registrarInfo("Template", "INICIANDO FLUXO FIXO para SHA: " + idSHA);

    // Etapa 1: Obter o dado externo (HOOK)
    double volume = obterLeitura(); 

    if (volume > 0.0) {
        // Etapa 2: Persistir a informação (HOOK)
        persistirLeitura();

        // Etapa 3: Verificar e Notificar o Limite (HOOK)
        verificarDispararAlerta();
    } else {
        Logger::getInstance()->registrarErro("Template", "Leitura de volume invalida (0.0). Processamento interrompido.");
    }

    Logger::getInstance()->registrarInfo("Template", "FLUXO CONCLUÍDO. Volume registrado: " + std::to_string(volume) + " m3.");
}


// =======================================================
// 2. Implementação da Classe Concreta (LeituraSHAProcessador)
// =======================================================

// Hook 1: A Leitura é delegada ao SubsistemaDados (que usa o Adapter)
double LeituraSHAProcessador::obterLeitura() {
    // Usa o executorDados injetado no construtor para chamar o método que usa o Adapter
    volumeLido = executorDados->obterLeituraVolume(idSHAAtual); 
    return volumeLido;
}

// Hook 2: A Persistência é delegada ao SubsistemaDados (que usa o DAO)
void LeituraSHAProcessador::persistirLeitura() {
    // Usa o executorDados injetado no construtor para chamar o método que salva
    executorDados->salvarConsumo(idSHAAtual, volumeLido);
}

// Hook 3: A Verificação é delegada ao SubsistemaAlerta
void LeituraSHAProcessador::verificarDispararAlerta() {
    // Usa o executorAlerta injetado no construtor para verificar o limite
    executorAlerta->verificarLimiteExcedido(idUsuarioAtual, idSHAAtual, volumeLido);
}