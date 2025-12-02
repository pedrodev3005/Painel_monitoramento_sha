// SubsistemaUsuarios.hpp

#ifndef SUBSISTEMA_USUARIOS_H
#define SUBSISTEMA_USUARIOS_H

#include "UsuarioDAO.hpp"  // Depende da interface
#include "Logger.hpp"    // Usa o serviço Singleton
#include <string>

/**
 * @brief Gerencia a lógica de negócio dos usuários e contas.
 * Atua como o cliente do Padrão DAO, traduzindo requisições de negócio em operações de persistência.
 */
class SubsistemaUsuarios {
private:
    UsuarioDAO* usuarioDAO; // Referência à interface DAO (baixo acoplamento)

public:
    // Construtor: Requer a injeção da interface DAO
    SubsistemaUsuarios(UsuarioDAO* dao);

    // Métodos de Lógica de Negócio (expostos à Fachada)
    bool criarUsuario(const Usuario& dados);
    Usuario buscarUsuarioComContas(int idUsuario);
    bool vincularHidrometro(int idUsuario, const std::string& idSHA);
};

#endif // SUBSISTEMA_USUARIOS_H