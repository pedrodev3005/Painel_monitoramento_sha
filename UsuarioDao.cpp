// UsuarioDAOImpl.cpp

#include "UsuarioDAO.hpp"
#include "DBConnection.hpp"
#include "Logger.hpp"
#include <sstream>

/**
 * @brief Implementação Concreta do UsuarioDAO.
 */
class UsuarioDAOImpl : public UsuarioDAO {
private:
    DBConnection* conexaoDB;

public:
    UsuarioDAOImpl(DBConnection* conn) : conexaoDB(conn) {
        // O Logger não é injetado, mas acessado via Singleton.
        Logger::getInstance()->registrarInfo("UsuarioDAOImpl", "Instanciado com conexao ao DB.");
    }

    bool salvar(const Usuario& usuario) override {
        std::string sql = "INSERT INTO usuarios (nome, cpf) VALUES (...)";
        conexaoDB->executarQuery(sql);
        
        Logger::getInstance()->registrarEventoCritico("UsuarioDAOImpl", "Usuario " + usuario.nome + " salvo.");
        return true;
    }

    Usuario buscarPorID(int idUsuario) override {
        // Simulação de busca no banco
        std::string sql = "SELECT * FROM usuarios WHERE idUsuario = " + std::to_string(idUsuario);
        conexaoDB->executarQuery(sql);
        
        Logger::getInstance()->registrarInfo("UsuarioDAOImpl", "Busca por ID: " + std::to_string(idUsuario));
        return Usuario{idUsuario, "Usuario Retornado", "000.000.000-00", {}};
    }

    bool vincularSHA(int idUsuario, const std::string& idSHA) override {
        std::string sql = "INSERT INTO Contas (idUsuario, idSHA) VALUES (...)";
        conexaoDB->executarQuery(sql);

        Logger::getInstance()->registrarInfo("UsuarioDAOImpl", "SHA " + idSHA + " vinculado ao ID " + std::to_string(idUsuario));
        return true;
    }

    std::vector<Conta> obterContasPorUsuario(int idUsuario) override {
        // Simulação de JOIN/busca para o RF 2.3 (Consumo Consolidado)
        std::string sql = "SELECT * FROM Contas WHERE idUsuario = " + std::to_string(idUsuario);
        conexaoDB->executarQuery(sql);
        
        // Simula o retorno de contas
        return {{"SHA123", idUsuario}, {"SHA456", idUsuario}};
    }
};