// LeitorImagemSHA.hpp

#ifndef LEITOR_IMAGEM_SHA_H
#define LEITOR_IMAGEM_SHA_H

#include "InterpretadorConsumo.hpp" // Usa struct Imagem e TipoMedidor
#include "Logger.hpp"

/**
 * @brief Implementa o Padrão Adapter para acessar a fonte externa de imagens do SHA.
 * É o único que conhece o diretório/API do SHA (cumpre a Restrição 1.2).
 */
class LeitorImagemSHA {
public:
    /**
     * @brief Obtém o arquivo de imagem do hidrômetro.
     */
    Imagem obterImagem(const std::string& idSHA) {
        Logger::getInstance()->registrarInfo("LeitorImagemSHA", "Buscando imagem para SHA: " + idSHA);
        return Imagem(
        idSHA, 
        "Buffer_Vazio_Simulado", 
        "/diretorio/sha/" + idSHA + ".bmp" // Caminho Simulado
        );
    }

    /**
     * @brief Determina o tipo do medidor para selecionar a Strategy.
     */
    TipoMedidor determinarTipoMedidor(const std::string& idSHA) {
        // Lógica para consultar DB ou metadados da imagem
        if (idSHA.find("DIG") != std::string::npos) {
            return TipoMedidor::DIGITAL;
        }
        return TipoMedidor::ANALOGICO;
    }
};

#endif // LEITOR_IMAGEM_SHA_H