// LeitorImagemSHA.hpp (O ADAPTER)

#ifndef LEITOR_IMAGEM_SHA_HPP
#define LEITOR_IMAGEM_SHA_HPP

#include "SHAConfigDAO.hpp" // Dependência para buscar o diretório da imagem no DB
#include "Logger.hpp"
#include "InterpretadorConsumo.hpp" // Para structs Imagem e enum TipoMedidor
#include "Entidades.hpp" // Para ConfiguracaoSHA, se necessário

#include <string>

/**
 * @brief Implementa o Padrão Adapter para acessar a fonte externa de imagens do SHA (Restrição 1.2).
 *
 * Esta classe é responsável por:
 * 1. Obter o caminho da imagem real do DAO.
 * 2. Utilizar a biblioteca de leitura de imagem (stb_image) para extrair o volume de uma ROI (Região de Interesse).
 * 3. Traduzir o resultado para o formato esperado pelo SubsistemaDados.
 */
class LeitorImagemSHA {
private:
    SHAConfigDAO* configDAO; // Injeção de Dependência

    /**
     * @brief Implementa a leitura de baixo nível do arquivo de imagem (BMP/PNG) para extrair o volume.
     * Simula o processo de OCR/ROI.
     * @param caminhoCompletoDaImagem O diretório e nome do arquivo da imagem.
     * @return O volume lido como double.
     */
    double lerROIReal(const std::string& caminhoCompletoDaImagem) const; 

public:
    /**
     * @brief Construtor que recebe a dependência do DAO de Configuração.
     */
    LeitorImagemSHA(SHAConfigDAO* dao);
    virtual ~LeitorImagemSHA() = default;

    /**
     * @brief Método principal que o SubsistemaDados chama para obter a leitura.
     * Orquestra o DAO e a leitura da imagem.
     */
    double obterLeituraVolume(const std::string& idSHA);

    /**
     * @brief Obtém o arquivo de imagem (apenas para simulação de buffer/metadados).
     */
    Imagem obterImagem(const std::string& idSHA);

    /**
     * @brief Determina o tipo do medidor para selecionar a Strategy (se aplicável).
     */
    TipoMedidor determinarTipoMedidor(const std::string& idSHA);
};

#endif // LEITOR_IMAGEM_SHA_HPP