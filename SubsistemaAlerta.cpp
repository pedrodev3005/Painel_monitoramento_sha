// SubsistemaAlerta.cpp

#include "SubsistemaAlerta.hpp"

bool SubsistemaAlerta::definirLimite(int idUsuario, const std::string& idSHA, double limiteVolumeM3) {
    // Chama o DAO com o NOVO parâmetro idSHA
    if (limiteDAO->salvarLimite(idUsuario, idSHA, limiteVolumeM3)) {
        Logger::getInstance()->registrarInfo("SubsistemaAlerta", "Limite de " + std::to_string(limiteVolumeM3) + 
                                            "m³ definido para SHA " + idSHA + " (Usuario " + std::to_string(idUsuario) + ").");
        return true;
    }
    return false;
}
void SubsistemaAlerta::notificarAlerta(int idUsuario, const AlertaConsumo& alerta, CanalAlerta canal) {
    //Usa a Factory para obter o objeto de envio correto
    ServicoNotificacao* notificador = notificadorFactory->criarNotificador(canal);

    if (notificador) {
        //Chama o método de envio (polimorfismo)
        notificador->enviarAlerta(alerta);
    }
}

// Método que será chamado pelo Template Method do Subsistema de Dados
bool SubsistemaAlerta::verificarLimiteExcedido(int idUsuario, const std::string& idSHA, double volumeAtual) {
    // Chama o DAO com o NOVO parâmetro idSHA para buscar o limite
    double limite = limiteDAO->buscarLimite(idUsuario, idSHA); 
    
    // Se o limite for 0.0 (padrão ou não encontrado), não disparamos alerta (ou usamos um limite padrão, 
    // mas vamos manter a lógica de usar o valor cadastrado).
    if (limite == 0.0) {
        Logger::getInstance()->registrarInfo("SubsistemaAlerta", "Limite nao encontrado para SHA " + idSHA + ". Alerta ignorado.");
        return false;
    }

    if (volumeAtual > limite) {
        Logger::getInstance()->registrarAlerta("SubsistemaAlerta", "LIMITE EXCEDIDO! SHA " + idSHA + ": " + 
                                               std::to_string(volumeAtual) + " m³ > Limite de " + std::to_string(limite) + " m³.");
        return true;
    }
    
    Logger::getInstance()->registrarInfo("SubsistemaAlerta", "Volume normal para SHA " + idSHA + " (" + 
                                         std::to_string(volumeAtual) + " m³).");
    return false;
}