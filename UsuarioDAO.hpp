// UsuarioDAO.hpp

#ifndef USUARIO_DAO_HPP
#define USUARIO_DAO_HPP

#include "Entidades.hpp"
#include <vector>
#include <string>

/**
 * @brief Interface (Contrato) do Padrão Data Access Object (DAO) para a Entidade Usuário.
 * * Define todas as operações de persistência (CRUD) que a aplicação necessita
 * para gerenciar usuários e as contas de hidrômetros vinculadas.
 */
class UsuarioDAO {
public:
    // Destrutor virtual: Essencial para classes base polimórficas (interfaces)
    virtual ~UsuarioDAO() = default;

    // --- Métodos de Persistência (Contrato) ---

    /**
     * @brief Salva um novo registro de usuário no RDB.
     * @param usuario Dados do usuário a ser salvo.
     * @return true se salvo com sucesso, false caso contrário.
     */
    virtual bool salvar(const Usuario& usuario) = 0;

    /**
     * @brief Busca um usuário pelo seu ID único.
     * @param idUsuario ID do usuário a ser buscado.
     * @return O objeto Usuario preenchido.
     */
    virtual Usuario buscarPorID(int idUsuario) = 0;

    /**
     * @brief Cria um mapeamento entre um usuário e um identificador de hidrômetro (SHA).
     * @param idUsuario ID do usuário.
     * @param idSHA Identificador do hidrômetro.
     * @return true se a vinculação for bem-sucedida.
     */
    virtual bool vincularSHA(int idUsuario, const std::string& idSHA) = 0;

    /**
     * @brief Obtém todas as contas (SHAs) vinculadas a um usuário específico.
     * (Essencial para o Consumo Consolidado - RF 2.3).
     * @param idUsuario ID do usuário.
     * @return Lista de objetos Conta.
     */
    virtual std::vector<Conta> obterContasPorUsuario(int idUsuario) = 0;

    // [Outros métodos CRUD como atualizar e deletar podem ser adicionados aqui, se necessário]
};

#endif // USUARIO_DAO_HPP