// DBConnection.h

#ifndef DBCONNECTION_HPP
#define DBCONNECTION_HPP

#include <string>
#include <stdexcept>
#include <iostream>
#include "sqlite3.h" // Inclui a biblioteca SQLite

// Inclui o Logger se ainda não estiver globalmente disponível
#include "Logger.hpp" 

/**
 * @brief Simula a conexão de baixo nível com o Banco de Dados.
 * Implementa a conexão real com SQLite para isolar a tecnologia de persistência.
 */
class DBConnection {
private:
    sqlite3* db; // Ponteiro real para a conexão SQLite
    const char* dbPath = "monitoramento.db"; 

    /**
     * @brief Cria todas as tabelas necessárias no banco de dados se elas não existirem.
     */
    void inicializarTabelas() {
        char *zErrMsg = 0;
        const char* sql_init = 
            // Tabela 1: Usuarios (RF 1)
            "CREATE TABLE IF NOT EXISTS Usuarios ("
            "idUsuario INTEGER PRIMARY KEY NOT NULL,"
            "nome TEXT NOT NULL,"
            "cpf TEXT NOT NULL); "
            
            // Tabela 2: Limites de Alerta (RF 3)
            "CREATE TABLE IF NOT EXISTS Limites ("
            "idUsuario INTEGER PRIMARY KEY NOT NULL,"
            "limiteVolumeM3 REAL NOT NULL); "
            
            // Tabela 3: Histórico de Leituras (RF 2)
            "CREATE TABLE IF NOT EXISTS Historico ("
            "idSHA TEXT NOT NULL,"
            "idUsuario INTEGER NOT NULL,"
            "volume REAL NOT NULL,"
            "dataHora INTEGER NOT NULL); "
            
            // Tabela 4: Configuração de SHA (NOVA - Vincula SHA a Diretório)
            "CREATE TABLE IF NOT EXISTS ConfiguracaoSHA ("
            "idSHA TEXT PRIMARY KEY NOT NULL,"
            "idUsuario INTEGER NOT NULL,"
            "diretorio TEXT NOT NULL,"
            "FOREIGN KEY(idUsuario) REFERENCES Usuarios(idUsuario)" // Garante integridade referencial
            ");";
        int rc_init = sqlite3_exec(db, sql_init, 0, 0, &zErrMsg);
        
        if (rc_init != SQLITE_OK) {
            Logger::getInstance()->registrarErro("DBConnection", "SQL Erro de Inicializacao: " + std::string(zErrMsg));
            sqlite3_free(zErrMsg);
        } else {
            Logger::getInstance()->registrarInfo("DBConnection", "Tabelas verificadas/criadas com sucesso.");
        }
    }

public:
    DBConnection() {
        // 1. Abre ou cria o arquivo DB
        if (sqlite3_open(dbPath, &db) != SQLITE_OK) {
            // Em caso de falha de abertura, lança exceção e registra
            std::string errMsg = "Nao foi possivel abrir o banco de dados: " + std::string(sqlite3_errmsg(db));
            Logger::getInstance()->registrarErro("DBConnection", errMsg);
            throw std::runtime_error(errMsg);
        }

        // 2. Inicializa as tabelas imediatamente após abrir a conexão
        inicializarTabelas();
    }
    
    ~DBConnection() {
        // Garante que a conexão seja fechada na destruição do objeto
        if (db) {
            sqlite3_close(db);
            Logger::getInstance()->registrarInfo("DBConnection", "Conexao com DB fechada.");
        }
    }

    /**
     * @brief Executa qualquer comando SQL (INSERT, UPDATE, DELETE, CREATE, etc.).
     */
    bool executarQuery(const std::string& sql) {
        char* errMsg = nullptr;
        // NULL é passado no callback para consultas não SELECT
        int rc = sqlite3_exec(db, sql.c_str(), nullptr, 0, &errMsg); 

        if (rc != SQLITE_OK) {
            Logger::getInstance()->registrarErro("DBConnection", "SQL Erro: " + std::string(errMsg) + " Query: " + sql);
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }
    
    // Método para execução de SELECTs (opcional, mas útil para o DAO)
    sqlite3* getDB() const { return db; }
};

#endif // DBCONNECTION_H