// EM LimiteAlertaDAO.cpp

#include "LimiteAlertaDAO.hpp"
#include <sstream>
#include <iostream>

// Variável temporária para armazenar o resultado do SELECT
static double limiteEncontrado = 0.0;

// Callback para ler o resultado do SELECT de limite
static int limiteCallback(void *data, int argc, char **argv, char **azColName) {
    if (argc > 0 && argv[0]) {
        limiteEncontrado = std::stod(argv[0]); // O primeiro campo retornado é o limite
    }
    return 0;
}

LimiteAlertaDAO::LimiteAlertaDAO(DBConnection* conn) : conexaoDB(conn) {
    Logger::getInstance()->registrarInfo("LimiteAlertaDAO", "DAO de Limite de Alerta inicializado.");
}

// 1. IMPLEMENTAÇÃO DE SALVAR/ATUALIZAR LIMITE (AGORA POR SHA)
bool LimiteAlertaDAO::salvarLimite(int idUsuario, const std::string& idSHA, double limiteVolumeM3) {
    
    std::stringstream ss;
    ss << "INSERT OR REPLACE INTO Limites (idUsuario, idSHA, limiteVolumeM3) VALUES ("
       << idUsuario << ", '"
       << idSHA << "', "
       << limiteVolumeM3 << ");";

    std::string sql = ss.str();
    
    if (conexaoDB->executarQuery(sql)) {
        Logger::getInstance()->registrarInfo("LimiteAlertaDAO", "Limite de " + std::to_string(limiteVolumeM3) + 
                                            "m³ SALVO/ATUALIZADO para SHA " + idSHA + " (Usuario " + std::to_string(idUsuario) + ").");
        return true;
    }
    
    Logger::getInstance()->registrarErro("LimiteAlertaDAO", "Falha ao salvar limite para SHA " + idSHA + ".");
    return false;
}

// 2. IMPLEMENTAÇÃO DE BUSCAR LIMITE (AGORA POR SHA)
double LimiteAlertaDAO::buscarLimite(int idUsuario, const std::string& idSHA) {
    limiteEncontrado = 0.0; // Reseta o resultado
    
    // SELECT que busca o limite correspondente à chave composta
    std::string sql = "SELECT limiteVolumeM3 FROM Limites WHERE idUsuario = " + 
                      std::to_string(idUsuario) + " AND idSHA = '" + idSHA + "';";
    
    char *zErrMsg = 0;
    
    int rc = sqlite3_exec(
        conexaoDB->getDB(), 
        sql.c_str(), 
        limiteCallback, // Callback que preenche limiteEncontrado
        0,              
        &zErrMsg
    );

    if (rc != SQLITE_OK) {
        Logger::getInstance()->registrarErro("LimiteAlertaDAO", "SQL Erro (SELECT Limite): " + std::string(zErrMsg));
        sqlite3_free(zErrMsg);
        return 0.0;
    }
    
    Logger::getInstance()->registrarInfo("LimiteAlertaDAO", "Limite " + std::to_string(limiteEncontrado) + " m³ buscado para SHA " + idSHA + ".");
    return limiteEncontrado;
}