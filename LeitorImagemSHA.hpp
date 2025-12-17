// LeitorImagemSHA.hpp (O ADAPTER)

#ifndef LEITOR_IMAGEM_SHA_HPP
#define LEITOR_IMAGEM_SHA_HPP

#include "SHAConfigDAO.hpp"
#include "Logger.hpp"
#include "Entidades.hpp" // Imagem, TipoMedidor, ConfiguracaoSHA

#include <string>

class LeitorImagemSHA {
private:
    SHAConfigDAO* configDAO;

    // Resolve qual arquivo de imagem deve ser lido dentro do diretório do SHA
    std::string resolverCaminhoImagemMaisRecente(const std::string& diretorio) const;

    // (Legado) leitura por pixel, só pra fallback
    double lerROIReal(const std::string& caminhoCompletoDaImagem) const;

public:
    explicit LeitorImagemSHA(SHAConfigDAO* dao);

    // (Legado) Mantido por compatibilidade. Ideal é usar Strategy no SubsistemaDados.
    double obterLeituraVolume(const std::string& idSHA);

    // Adapter principal: retorna o caminho real do arquivo de imagem
    Imagem obterImagem(const std::string& idSHA);

    // Heurística simples: decide Strategy (DIGITAL/ANALOGICO)
    TipoMedidor determinarTipoMedidor(const std::string& idSHA);
};

#endif // LEITOR_IMAGEM_SHA_HPP
