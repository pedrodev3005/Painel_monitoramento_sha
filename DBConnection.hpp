// DBConnection.hpp

#ifndef DBCONNECTION_H
#define DBCONNECTION_H

#include <string>

/**
 * @brief Simula a conexão de baixo nível com o Banco de Dados.
 */
class DBConnection {
public:
    // Simula a execução de qualquer comando SQL
    bool executarQuery(const std::string& sql) {
        // Lógica de conexão e execução de SQL
        // Retorna true simulando sucesso
        return true; 
    }
};

#endif // DBCONNECTION_H