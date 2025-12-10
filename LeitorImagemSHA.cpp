// LeitorImagemSHA.cpp

#include "LeitorImagemSHA.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <filesystem> // <-- NOVO: Biblioteca para manipular diretórios
#include <algorithm>
#include <stdexcept>
#include <regex>       // Para buscar o índice numérico

namespace fs = std::filesystem;

// Diretório base 
const std::string BASE_DIR = "C:/Users/pedro/Documents/SHAs/"; 

Imagem LeitorImagemSHA::obterImagem(const std::string& idSHA) {
    std::string caminhoDiretorio = BASE_DIR + idSHA + "/";
    std::string arquivoMaisRecente = "";
    int maiorIndice = -1;
    
    // Expressão regular para encontrar "hidrometro_[NUMERO].bmp"
    // Captura o número no primeiro grupo (Grupo 1)
    std::regex padrao("hidrometro_(\\d+)\\.bmp");
    
    Logger::getInstance()->registrarInfo("LeitorImagemSHA", "Varrendo diretório: " + caminhoDiretorio);
    
    try {
        // Itera sobre todos os itens no diretório do SHA
        for (const auto& entrada : fs::directory_iterator(caminhoDiretorio)) {
            if (entrada.is_regular_file()) {
                std::string nomeArquivo = entrada.path().filename().string();
                std::smatch match;

                // Tenta casar o nome do arquivo com o padrão
                if (std::regex_match(nomeArquivo, match, padrao)) {
                    
                    // match[1] contém o número (o índice)
                    int indiceAtual = std::stoi(match[1].str()); 

                    if (indiceAtual > maiorIndice) {
                        maiorIndice = indiceAtual;
                        arquivoMaisRecente = nomeArquivo;
                    }
                }
            }
        }

        if (maiorIndice != -1) {
            std::string caminhoCompleto = caminhoDiretorio + arquivoMaisRecente;
            Logger::getInstance()->registrarInfo("LeitorImagemSHA", "Arquivo encontrado (Indice Max): " + arquivoMaisRecente);
            
            // Retorna o DTO Imagem com o caminho correto para o Strategy
            return Imagem(
                idSHA, 
                "Buffer_Lido", 
                caminhoCompleto
            );
        }

    } catch (const fs::filesystem_error& e) {
        Logger::getInstance()->registrarErro("LeitorImagemSHA", "Erro de acesso ao FS: " + std::string(e.what()));
    }
    
    Logger::getInstance()->registrarErro("LeitorImagemSHA", "Falha ao encontrar o arquivo mais recente para SHA " + idSHA + ".");
    return Imagem(idSHA, "BUFFER_NULL", caminhoDiretorio); // Retorno de falha
}