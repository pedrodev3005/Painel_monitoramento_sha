// SHAConfigDAO.h

#ifndef SHA_CONFIG_DAO_HPP
#define SHA_CONFIG_DAO_HPP

#include "DBConnection.hpp"
#include "Entidades.hpp"
#include "Logger.hpp"
#include <vector>

/**
 * @brief DAO responsável pela persistência e recuperação das configurações de SHA
 * (vinculação entre idSHA, idUsuario e o diretório físico).
 */
class SHAConfigDAO {
private:
    DBConnection* conexaoDB;

public:
    SHAConfigDAO(DBConnection* conn);

    /**
     * @brief Salva ou atualiza a configuração de um SHA (idSHA -> diretorio) no DB.
     */
    bool salvarConfiguracao(const ConfiguracaoSHA& config);

    /**
     * @brief Busca a configuração completa de um SHA específico.
     * Necessário para o Adapter saber qual diretório usar.
     */
    ConfiguracaoSHA buscarConfiguracao(const std::string& idSHA);

    /**
     * @brief Lista todas as configurações de SHA ativas no sistema.
     * Essencial para o modo de Varredura Automática.
     */
    std::vector<ConfiguracaoSHA> listarTodosAtivos();
};

#endif // SHA_CONFIG_DAO_H