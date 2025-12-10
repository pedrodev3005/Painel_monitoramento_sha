// SHAConfigDAO.cpp

#include "SHAConfigDAO.hpp"
#include <sstream>
#include <iostream>

// Variável global temporária para guardar o resultado do SELECT.
// SQLite callbacks são difíceis, este é um hack comum em pequenos projetos.
static ConfiguracaoSHA ultimoResultadoSHA; 

// =======================================================
// Funções de Callback do SQLite para a Configuração
// =======================================================

// Callback para ler o resultado de um SELECT de ConfiguracaoSHA
static int shaConfigCallback(void *data, int argc, char **argv, char **azColName) {
    // Presume que a query retornou idSHA, idUsuario, diretorio
    if (argc >= 3) {
        // argv[0] = idSHA, argv[1] = idUsuario, argv[2] = diretorio
        ultimoResultadoSHA.idSHA = argv[0] ? argv[0] : "";
        ultimoResultadoSHA.idUsuario = argv[1] ? std::stoi(argv[1]) : 0;
        ultimoResultadoSHA.diretorio = argv[2] ? argv[2] : "";
    }
    return 0; // Continuar processando linhas
}

// =======================================================
// Implementação dos Métodos DAO
// =======================================================

SHAConfigDAO::SHAConfigDAO(DBConnection* conn) : conexaoDB(conn) {
    Logger::getInstance()->registrarInfo("SHAConfigDAO", "DAO de Configuracao SHA inicializado.");
}

bool SHAConfigDAO::salvarConfiguracao(const ConfiguracaoSHA& config) {
    // Monta a query para inserir a configuração, usando REPLACE para atualizar se já existir
    std::string sql = "INSERT OR REPLACE INTO ConfiguracaoSHA (idSHA, idUsuario, diretorio) VALUES ('" + 
                       config.idSHA + "', " + 
                       std::to_string(config.idUsuario) + ", '" + 
                       config.diretorio + "');";
    
    if (conexaoDB->executarQuery(sql)) {
        Logger::getInstance()->registrarInfo("SHAConfigDAO", "Configuracao SHA " + config.idSHA + " SALVA/ATUALIZADA no DB.");
        return true;
    }
    
    Logger::getInstance()->registrarErro("SHAConfigDAO", "Falha ao salvar configuracao do SHA " + config.idSHA + ".");
    return false;
}

ConfiguracaoSHA SHAConfigDAO::buscarConfiguracao(const std::string& idSHA) {
    ultimoResultadoSHA = ConfiguracaoSHA(); // Limpa o resultado anterior
    
    std::string sql = "SELECT idSHA, idUsuario, diretorio FROM ConfiguracaoSHA WHERE idSHA = '" + idSHA + "';";
    char *zErrMsg = 0;
    
    int rc = sqlite3_exec(
        conexaoDB->getDB(), 
        sql.c_str(), 
        shaConfigCallback, // Nossa função de leitura de linha
        0,                 // Argumento passado ao callback
        &zErrMsg
    );

    if (rc != SQLITE_OK) {
        Logger::getInstance()->registrarErro("SHAConfigDAO", "SQL Erro (SELECT): " + std::string(zErrMsg));
        sqlite3_free(zErrMsg);
        return ConfiguracaoSHA(); // Retorna vazio em caso de erro
    }
    
    // Se a query foi bem sucedida, o callback preencheu 'ultimoResultadoSHA'
    return ultimoResultadoSHA;
}

std::vector<ConfiguracaoSHA> SHAConfigDAO::listarTodosAtivos() {
    // ** SIMULAÇÃO SIMPLES para fins de demonstração (não implementa SELECT de múltiplos registros)**
    // Em um projeto real, esta função usaria uma API de SELECT mais complexa ou uma classe SQLite Wrapper.
    
    // Retorna uma lista com a configuração que acabamos de salvar.
    // Para testar, vamos retornar um SHA fixo, que assumimos estar no DB.
    
    Logger::getInstance()->registrarInfo("SHAConfigDAO", "Simulando SELECT de todos os SHAs ativos...");
    
    std::vector<ConfiguracaoSHA> listaAtivos;
    listaAtivos.push_back(ConfiguracaoSHA("SHA-DIG-456", 101, "C:/Users/pedro/Documents/SHAs/SHA-DIG-456/"));
    listaAtivos.push_back(ConfiguracaoSHA("SHA-ANA-100", 101, "C:/Users/pedro/Documents/SHAs/SHA-ANA-100/"));
    
    return listaAtivos;
}