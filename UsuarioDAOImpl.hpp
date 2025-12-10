// UsuarioDAOImpl.hpp

#ifndef USUARIO_DAO_IMPL_HPP
#define USUARIO_DAO_IMPL_HPP

#include "UsuarioDAO.hpp" // A interface que esta classe implementa
#include "DBConnection.hpp" // Dependencia de baixo nivel
#include "Logger.hpp"     // Servico Singleton

/**
 * @brief Implementação Concreta do UsuarioDAO.
 * Contém a lógica real de acesso ao RDB (SQL/ORM simulado).
 * Esta classe é o ponto onde o código de persistência reside.
 */
class UsuarioDAOImpl : public UsuarioDAO {
private:
    DBConnection* conexaoDB;

public:
    /**
     * @brief Construtor que recebe a dependência da conexão (DBConnection).
     */
    UsuarioDAOImpl(DBConnection* conn);

    // --- Implementação dos Métodos da Interface (Override) ---
    std::vector<Usuario> listarTodos(); // <-- NOVO MÉTODO

    bool salvar(const Usuario& usuario) override;
    Usuario buscarPorID(int idUsuario) override;
    bool vincularSHA(int idUsuario, const std::string& idSHA) override;
    std::vector<std::string> obterContasPorUsuario(int idUsuario) override;
};

// Se você decidir colocar a implementação de construtores e métodos simples aqui (apenas no cabeçalho):
/*
UsuarioDAOImpl::UsuarioDAOImpl(DBConnection* conn) : conexaoDB(conn) {
    if (!conn) {
        Logger::getInstance()->registrarErro("UsuarioDAOImpl", "ConexaoDB nula ao inicializar DAO.");
        throw std::runtime_error("Conexao DB nao pode ser nula.");
    }
    Logger::getInstance()->registrarInfo("UsuarioDAOImpl", "DAO de Usuários inicializado com sucesso.");
}
*/

#endif // USUARIO_DAO_IMPL_HPP