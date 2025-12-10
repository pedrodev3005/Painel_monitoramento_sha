// UsuarioDAOImpl.cpp

#include "UsuarioDAOImpl.hpp" // Define a classe concreta
#include <sstream>
#include <stdexcept>

// =======================================================
// Implementação dos Métodos
// =======================================================

// Variável global para armazenar temporariamente o resultado do SELECT do SQLite
static std::vector<Usuario> listaTemporariaUsuarios;

UsuarioDAOImpl::UsuarioDAOImpl(DBConnection* conn) : conexaoDB(conn) {
    if (!conn) {
        Logger::getInstance()->registrarErro("UsuarioDAOImpl", "ConexaoDB nula ao inicializar DAO.");
        throw std::runtime_error("Conexao DB nao pode ser nula.");
    }
    Logger::getInstance()->registrarInfo("UsuarioDAOImpl", "DAO de Usuários inicializado com sucesso.");
}

bool UsuarioDAOImpl::salvar(const Usuario& usuario) {
    // 1. Simulação da query SQL
    std::string sql = "INSERT INTO Usuarios (nome, cpf) VALUES ('" + usuario.nome + "', '" + usuario.cpf + "')";
    
    // 2. Execução da query
    if (conexaoDB->executarQuery(sql)) {
        Logger::getInstance()->registrarEventoCritico("UsuarioDAOImpl", "Usuario " + usuario.nome + " salvo com sucesso.");
        return true;
    }
    
    Logger::getInstance()->registrarErro("UsuarioDAOImpl", "Falha ao salvar usuário no RDB.");
    return false;
}

Usuario UsuarioDAOImpl::buscarPorID(int idUsuario) {
    // Simulação da busca no DB
    std::string sql = "SELECT nome, cpf FROM Usuarios WHERE idUsuario = " + std::to_string(idUsuario);
    conexaoDB->executarQuery(sql);
    
    // Simula o retorno de um objeto preenchido
    Usuario u{idUsuario, "Usuario Retornado", "000.000.000-00", {}};
    
    Logger::getInstance()->registrarInfo("UsuarioDAOImpl", "Busca de Usuario ID: " + std::to_string(idUsuario));
    return u;
}

bool UsuarioDAOImpl::vincularSHA(int idUsuario, const std::string& idSHA) {
    // Simulação da query para criar a relação (Chave Estrangeira)
    std::string sql = "INSERT INTO Contas (idUsuario, idSHA) VALUES (" + std::to_string(idUsuario) + ", '" + idSHA + "')";
    
    if (conexaoDB->executarQuery(sql)) {
        Logger::getInstance()->registrarInfo("UsuarioDAOImpl", "SHA " + idSHA + " vinculado.");
        return true;
    }
    
    Logger::getInstance()->registrarErro("UsuarioDAOImpl", "Falha ao vincular SHA: " + idSHA);
    return false;
}

std::vector<std::string> UsuarioDAOImpl::obterContasPorUsuario(int idUsuario) {
    // 1. Simulação do SELECT para obter as contas (IDs de SHA)
    // Nota: Em um sistema real, você faria um JOIN na tabela ConfiguracaoSHA
    std::string sql = "SELECT idSHA FROM ConfiguracaoSHA WHERE idUsuario = " + std::to_string(idUsuario) + ";";
    conexaoDB->executarQuery(sql);
    
    // 2. LÓGICA DE RETORNO CORRIGIDA:
    
    Logger::getInstance()->registrarInfo("UsuarioDAOImpl", "Obtidas contas (simuladas) para ID: " + std::to_string(idUsuario));
    
    // A função deve retornar apenas std::string
    return {
        "SHA-DIG-456", // SHA ID 1 (string)
        "SHA-ANA-100"  // SHA ID 2 (string)
    };
}

// Callback para SELECT ALL (preenche listaTemporariaUsuarios)

static int listarTodosCallback(void *data, int argc, char **argv, char **azColName) {
    if (argc >= 3) {
        // Agora, o construtor Usuario(int, const char*, const char*) será chamado
        listaTemporariaUsuarios.emplace_back(
            std::stoi(argv[0]),   // int id
            argv[1],              // const char* nome
            argv[2]               // const char* cpf
        );
    }
    return 0;
}


std::vector<Usuario> UsuarioDAOImpl::listarTodos() {
    listaTemporariaUsuarios.clear(); // Limpa o vetor global antes de cada SELECT

    const char* sql = "SELECT idUsuario, nome, cpf FROM Usuarios;";
    char *zErrMsg = 0;
    
    // Execução da consulta
    int rc = sqlite3_exec(conexaoDB->getDB(), sql, listarTodosCallback, 0, &zErrMsg); 

    if (rc != SQLITE_OK) {
        Logger::getInstance()->registrarErro("UsuarioDAOImpl", "SQL Erro (SELECT ALL): " + std::string(zErrMsg));
        sqlite3_free(zErrMsg);
        return {}; // Retorna vetor vazio em caso de erro
    }
    
    // CORREÇÃO: Retornar a variável global que o callback preencheu
    return listaTemporariaUsuarios;
}