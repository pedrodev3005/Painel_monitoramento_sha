// ConsumoHistoricoDAO.cpp

#include "ConsumoHistoricoDAO.hpp"
#include <sstream>



ConsumoHistoricoDAO::ConsumoHistoricoDAO(DBConnection* conn) : conexaoDB(conn) {
    Logger::getInstance()->registrarInfo("ConsumoHistoricoDAO", "DAO de histórico inicializado.");
}

// =======================================================
// Implementação: Salvar Leitura (Usado pelo Template Method)
// =======================================================

bool ConsumoHistoricoDAO::salvarLeitura(int idUsuario, const LeituraConsumo& leitura) {
    // Monta a query SQL para inserir os dados, incluindo o idUsuario
    std::string sql = "INSERT INTO Historico (idSHA, idUsuario, volume, dataHora) VALUES ('" + 
                       leitura.idSHA + "', " + 
                       std::to_string(idUsuario) + ", " + 
                       std::to_string(leitura.volume) + ", " + 
                       std::to_string(leitura.timestamp) + ");";
    
    // Execução da query
    if (conexaoDB->executarQuery(sql)) { 
        Logger::getInstance()->registrarInfo("ConsumoHistoricoDAO", "Leitura " + leitura.idSHA + " SALVA NO DB.");
        return true;
    }
    
    Logger::getInstance()->registrarErro("ConsumoHistoricoDAO", "Falha ao salvar Leitura no Historico.");
    return false;
}

// =======================================================
// Implementação: Agregação (Simulação de SELECT SUM)
// =======================================================

ConsumoDTO ConsumoHistoricoDAO::agregarConsumoPorPeriodo(int idUsuario, std::time_t inicio, std::time_t fim) {
    // Simulação da query SQL que somaria o consumo para o usuário e período
    std::string sql = "SELECT SUM(volume) FROM Historico WHERE idUsuario = " + std::to_string(idUsuario) + 
                       " AND dataHora BETWEEN " + std::to_string(inicio) + " AND " + std::to_string(fim) + ";";
    
    // Execução da query (apenas para logging e simulação de uso do DB)
    conexaoDB->executarQuery(sql);
    
    // Em um sistema real, aqui haveria lógica para ler o resultado da agregação.
    // Retorno simulado:
    ConsumoDTO resultado = {150.75}; // Valor fixo para teste
    
    Logger::getInstance()->registrarInfo("ConsumoHistoricoDAO", "Consulta consolidada de consumo para ID: " + std::to_string(idUsuario));
    return resultado;
}