// TemplateMonitoramento.cpp

#include "TemplateMonitoramento.hpp"
#include <stdexcept>
#include "SubsistemaAlerta.hpp" 
#include "Entidades.hpp"   // <-- CORREÇÃO 2: Contém a definição da struct Imagem
#include <sstream>
#include <fstream>         // <-- CORREÇÃO 1: Contém a definição de std::ifstream
#include <cstdlib>
#include <ctime>
#include <limits>

// =======================================================
// Implementação da Classe Base (Fluxo Fixo)
// =======================================================

// método Template é implementado aqui
void FluxoProcessamentoBase::executarFluxo(const std::string& idSHA, int idUsuario, 
                                          LeitorImagemSHA* leitor, ConsumoHistoricoDAO* dao, 
                                          SubsistemaAlerta* alerta) {
    
    // 1. Obter Imagem 
    Logger::getInstance()->registrarInfo("Template", "INICIANDO FLUXO: " + idSHA);
    
    // O Adaptador é usado para buscar a imagem (cumpre a restrição SHA)
    Imagem imagem = leitor->obterImagem(idSHA);
    
    // 2. Extrair Consumo (Hook Abstrato - Implementado pela Strategy)
   double volume = extrairConsumo(imagem);
    try {
        volume = extrairConsumo(imagem); // Chama o Hook da subclasse
    } catch (const std::exception& e) {
        Logger::getInstance()->registrarErro("Template", "Falha na extração (OCR): " + std::string(e.what()));
        return; // Falha na leitura, aborta o fluxo
    }

    // 3. Salvar Histórico 
    LeituraConsumo leitura = {idSHA, volume, std::time(nullptr)};
    salvarHistorico(idUsuario, leitura, dao); // CORRIGIDO: Passando idUsuario

    // 4. Verificar Alerta (Hook Abstrato - Implementado pelo Subsistema Alerta)
    verificarLimite(idUsuario, idSHA, volume);
    
    Logger::getInstance()->registrarInfo("Template", "FLUXO CONCLUÍDO: Volume: " + std::to_string(volume) + " m3.");
}

// =======================================================
// Implementação da Classe Concreta (LeituraSHAProcessador)
// =======================================================

// Hook 1: Lógica de Extração de Consumo (usa o Padrão Strategy)
// TemplateMonitoramento.cpp (Dentro do Hook extrairConsumo)

// EM TemplateMonitoramento.cpp (Dentro de LeituraSHAProcessador::extrairConsumo)

double LeituraSHAProcessador::extrairConsumo(const Imagem& imagem) {
    // Note: Assumindo que a struct Imagem tem o campo 'caminhoArquivo'
    std::ifstream inputFile(imagem.caminhoArquivo); 
    std::string volumeString;
    double volumeReal = 0.0; // Inicializa com valor padrão

    if (inputFile.is_open()) {
        std::getline(inputFile, volumeString);
        inputFile.close();
        
        try {
            // Conversão da string lida para double
            volumeReal = std::stod(volumeString);
        } catch (const std::exception& e) {
            Logger::getInstance()->registrarErro("Strategy", "Erro de conversão de string: " + std::string(e.what()));
            return 0.0; // Retorna 0.0 em caso de erro de leitura numérica
        }
        
        // Lógica de Strategy
        if (imagem.idSHA.find("DIG") != std::string::npos) { // <-- AGORA idSHA ESTÁ DEFINIDO
            Logger::getInstance()->registrarInfo("Strategy:Digital", "Processamento digital aplicado.");
            return volumeReal;
        } else {
            Logger::getInstance()->registrarInfo("Strategy:Analogico", "Processamento analogico aplicado.");
            return volumeReal * 0.95; 
        }

    } else {
        Logger::getInstance()->registrarErro("Strategy", "Nao foi possivel ler o volume do arquivo: " + imagem.caminhoArquivo);
        // CORREÇÃO 3: Retorno explícito para o caso de arquivo não aberto
        return 0.0; 
    }
}

// Hook 2: Lógica de Verificação de Limite (usa o SubsistemaAlerta)
void LeituraSHAProcessador::verificarLimite(int idUsuario, const std::string& idSHA, double volume) {
    // O Template delega a responsabilidade de verificação e notificação para o SubsistemaAlerta
    subsistemaAlerta->verificarLimiteExcedido(idUsuario, idSHA, volume);
}
