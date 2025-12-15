#ifndef LIMITE_ALERTA_DAO_HPP
#define LIMITE_ALERTA_DAO_HPP

#include "DBConnection.hpp" 
#include "Logger.hpp"

class LimiteAlertaDAO {
private:
    DBConnection* conexaoDB;

public:
    LimiteAlertaDAO(DBConnection* conn); 

    /**
     * @brief Salva ou atualiza o limite de volume para um SHA específico de um usuário.
     * @param idSHA O identificador do medidor.
     */
    bool salvarLimite(int idUsuario, const std::string& idSHA, double limiteVolumeM3); // <-- ASSINATURA ATUALIZADA

    /**
     * @brief Busca o limite de volume configurado para um SHA específico.
     * @param idSHA O identificador do medidor.
     * @return O limite em m3, ou 0.0 se não encontrado.
     */
    double buscarLimite(int idUsuario, const std::string& idSHA); // <-- ASSINATURA ATUALIZADA
};

#endif // LIMITE_ALERTA_DAO_H