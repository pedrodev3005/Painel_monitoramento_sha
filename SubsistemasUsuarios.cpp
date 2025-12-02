// SubsistemaUsuarios.cpp

#include "SubsistemaUsuarios.hpp"
#include <stdexcept>

// Construtor (simplesmente armazena a referência injetada)
SubsistemaUsuarios::SubsistemaUsuarios(UsuarioDAO* dao) : usuarioDAO(dao) {
    Logger::getInstance()->registrarInfo("SubsistemaUsuarios", "Subsistema inicializado.");
}

bool SubsistemaUsuarios::criarUsuario(const Usuario& dados) {
    if (dados.cpf.empty() || dados.nome.empty()) {
        Logger::getInstance()->registrarErro("SubsistemaUsuarios", "Tentativa de criar usuario com dados incompletos.");
        return false;
    }
    
    // 1. Lógica de Negócio: Validação
    if (dados.cpf.length() < 11) {
        Logger::getInstance()->registrarErro("SubsistemaUsuarios", "CPF invalido para criacao.");
        return false;
    }

    // 2. Delegação ao DAO (Persistência)
    bool sucesso = usuarioDAO->salvar(dados);

    if (sucesso) {
        Logger::getInstance()->registrarEventoCritico("SubsistemaUsuarios", "NOVO USUARIO CRIADO: " + dados.nome);
    }
    return sucesso;
}

Usuario SubsistemaUsuarios::buscarUsuarioComContas(int idUsuario) {
    // 1. Delegação de busca principal
    Usuario u = usuarioDAO->buscarPorID(idUsuario);

    if (u.idUsuario != 0) {
        // 2. Delegação de busca secundária (consolidação de dados)
        u.contas = usuarioDAO->obterContasPorUsuario(idUsuario);
        Logger::getInstance()->registrarInfo("SubsistemaUsuarios", "Usuario ID " + std::to_string(idUsuario) + " e contas obtidos.");
    }
    return u;
}

bool SubsistemaUsuarios::vincularHidrometro(int idUsuario, const std::string& idSHA) {
    // Lógica de verificação antes de vincular, depois delega:
    bool sucesso = usuarioDAO->vincularSHA(idUsuario, idSHA);
    
    if (sucesso) {
        Logger::getInstance()->registrarInfo("SubsistemaUsuarios", "SHA " + idSHA + " vinculado.");
    }
    return sucesso;
}