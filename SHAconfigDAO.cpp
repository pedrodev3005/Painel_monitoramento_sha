// SHAConfigDAO.cpp

#include "SHAConfigDAO.hpp"
#include "Logger.hpp"

#include "sqlite3.h"
#include <string>
#include <vector>

SHAConfigDAO::SHAConfigDAO(DBConnection* conn) : conexaoDB(conn) {
    Logger::getInstance()->registrarInfo("SHAConfigDAO", "DAO de Configuracao SHA inicializado.");
}

bool SHAConfigDAO::salvarConfiguracao(const ConfiguracaoSHA& config) {
    // Mantive teu INSERT OR REPLACE porque já funciona no teu padrão
    std::string sql =
        "INSERT OR REPLACE INTO ConfiguracaoSHA (idSHA, idUsuario, diretorio) VALUES ('" +
        config.idSHA + "', " +
        std::to_string(config.idUsuario) + ", '" +
        config.diretorio + "');";

    if (conexaoDB->executarQuery(sql)) {
        Logger::getInstance()->registrarInfo("SHAConfigDAO",
            "Configuracao SHA " + config.idSHA + " SALVA/ATUALIZADA no DB.");
        return true;
    }

    Logger::getInstance()->registrarErro("SHAConfigDAO",
        "Falha ao salvar configuracao do SHA " + config.idSHA + ".");
    return false;
}

ConfiguracaoSHA SHAConfigDAO::buscarConfiguracao(const std::string& idSHA) {
    // Retorno padrão “não encontrado”
    ConfiguracaoSHA result("", -1, "");

    sqlite3* db = conexaoDB->getDB();
    const char* sql = "SELECT idSHA, idUsuario, diretorio FROM ConfiguracaoSHA WHERE idSHA = ?;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::getInstance()->registrarErro("SHAConfigDAO",
            std::string("Falha prepare buscarConfiguracao: ") + sqlite3_errmsg(db));
        return result;
    }

    sqlite3_bind_text(stmt, 1, idSHA.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        const unsigned char* c0 = sqlite3_column_text(stmt, 0);
        int c1 = sqlite3_column_int(stmt, 1);
        const unsigned char* c2 = sqlite3_column_text(stmt, 2);

        result.idSHA = c0 ? reinterpret_cast<const char*>(c0) : "";
        result.idUsuario = c1;
        result.diretorio = c2 ? reinterpret_cast<const char*>(c2) : "";
    }
    // se não achou, mantém idUsuario=-1

    sqlite3_finalize(stmt);
    return result;
}

std::vector<ConfiguracaoSHA> SHAConfigDAO::listarTodosAtivos() {
    std::vector<ConfiguracaoSHA> lista;

    sqlite3* db = conexaoDB->getDB(); // ✅ aqui é conexaoDB (não "conn")
    const char* sql = "SELECT idSHA, idUsuario, diretorio FROM ConfiguracaoSHA;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        Logger::getInstance()->registrarErro("SHAConfigDAO",
            std::string("Falha prepare listarTodosAtivos: ") + sqlite3_errmsg(db));
        return lista;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* c0 = sqlite3_column_text(stmt, 0);
        int c1 = sqlite3_column_int(stmt, 1);
        const unsigned char* c2 = sqlite3_column_text(stmt, 2);

        std::string id = c0 ? reinterpret_cast<const char*>(c0) : "";
        std::string dir = c2 ? reinterpret_cast<const char*>(c2) : "";

        lista.emplace_back(id, c1, dir);
    }

    sqlite3_finalize(stmt);
    return lista;
}
