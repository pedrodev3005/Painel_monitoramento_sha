// LeitorImagemSHA.cpp (O ADAPTER - Implementação da Leitura de Imagem)

#include "LeitorImagemSHA.hpp"
#include <sstream>
#include <fstream>
#include <cmath> // Para std::round

// =====================================================================
// INCLUSÃO E IMPLEMENTAÇÃO DO STB_IMAGE.H
// Define a implementação da biblioteca header-only para leitura de imagem
// (Deve estar presente em APENAS UM arquivo .cpp do projeto)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// =====================================================================


// Construtor
LeitorImagemSHA::LeitorImagemSHA(SHAConfigDAO* dao) : configDAO(dao) {
    Logger::getInstance()->registrarInfo("LeitorImagemSHA", "Adapter inicializado com DAO (Usando stb_image para leitura de pixels).");
}

// ----------------------------------------------------------------------
// Implementação da Leitura Real de ROI (Simulada por Leitura de Pixel)
// ----------------------------------------------------------------------

/**
 * @brief Simula a leitura da Região de Interesse (ROI) da imagem, 
 * lendo o valor de um pixel específico como base para o volume extraído.
 * @param caminhoCompletoDaImagem O caminho real do arquivo BMP/PNG.
 * @return O volume lido como double.
 */
double LeitorImagemSHA::lerROIReal(const std::string& caminhoCompletoDaImagem) const {
    Logger::getInstance()->registrarInfo("LeitorImagemSHA", "Iniciando leitura REAL da imagem: " + caminhoCompletoDaImagem);

    int largura, altura, canais;
    // Carrega a imagem. stb_image pode ler BMP, PNG, JPG, etc.
    unsigned char *data = stbi_load(caminhoCompletoDaImagem.c_str(), &largura, &altura, &canais, 0);

    if (!data) {
        // stbi_failure_reason() fornece o motivo da falha.
        Logger::getInstance()->registrarErro("LeitorImagemSHA", "Falha ao carregar imagem: " + std::string(stbi_failure_reason()));
        return 0.0;
    }

    // --- DEFINIÇÃO DA ROI (Região de Interesse) ---
    // Coordenadas simuladas para o centro de um dígito no hidrômetro.
    int x_roi = 150;
    int y_roi = 70;

    if (x_roi >= largura || y_roi >= altura) {
        Logger::getInstance()->registrarErro("LeitorImagemSHA", "Coordenadas da ROI (" + std::to_string(x_roi) + ", " + std::to_string(y_roi) + ") fora dos limites da imagem.");
        stbi_image_free(data);
        return 0.0;
    }
    
    // Calcula o índice do pixel na memória (Ex: componente Vermelho/Primeiro Canal)
    int indice = (y_roi * largura + x_roi) * canais;
    int valor_pixel = data[indice]; // Valor de 0 a 255
    
    // Libera a memória da imagem
    stbi_image_free(data);
    
    // --- CONVERSÃO PARA O VOLUME (SIMULAÇÃO DETERMINÍSTICA) ---
    // Usa o valor do pixel lido como base para um volume determinístico e realista.
    // Garante que a leitura muda se a imagem (e o pixel) mudar.
    double volume_base = 100.00 + (double)valor_pixel; // Volume base + valor do pixel
    double volume = std::round(volume_base * 10.0) / 10.0; // Arredonda para 1 casa decimal
    
    Logger::getInstance()->registrarInfo("LeitorImagemSHA", "Pixel da ROI (" + std::to_string(x_roi) + ", " + std::to_string(y_roi) + 
                                         ") lido (Valor: " + std::to_string(valor_pixel) + "). Volume extraído: " + std::to_string(volume) + " m3");

    return volume;
}


// ----------------------------------------------------------------------
// Método Público Principal (Chamado pelo SubsistemaDados)
// ----------------------------------------------------------------------

double LeitorImagemSHA::obterLeituraVolume(const std::string& idSHA) {
    ConfiguracaoSHA config = configDAO->buscarConfiguracao(idSHA);

    if (config.idUsuario == -1) {
        Logger::getInstance()->registrarErro("LeitorImagemSHA", "Configuracao nao encontrada para o ID: " + idSHA);
        return 0.0;
    }
    
    // Constrói o caminho completo da imagem BMP/PNG
    // Nome do arquivo de imagem deve estar no diretório configurado
    std::string caminhoDaImagem = config.diretorio + "leitura_hidrometro.bmp";
    
    // Chama a função real de processamento de imagem
    return lerROIReal(caminhoDaImagem);
}

// ----------------------------------------------------------------------
// Outros Métodos (Manutenção da Interface)
// ----------------------------------------------------------------------

// Implementação do construtor (definido no .h)
// LeitorImagemSHA::LeitorImagemSHA(SHAConfigDAO* dao) ... (já implementado no topo)

Imagem LeitorImagemSHA::obterImagem(const std::string& idSHA) {
    ConfiguracaoSHA config = configDAO->buscarConfiguracao(idSHA);
    std::string caminho = (config.idUsuario != -1) ? config.diretorio + "leitura_hidrometro.bmp" : "Caminho_Invalido";
    
    return Imagem(
        idSHA, 
        "Buffer_Simulado", 
        caminho
    );
}

TipoMedidor LeitorImagemSHA::determinarTipoMedidor(const std::string& idSHA) {
    if (idSHA.find("DIG") != std::string::npos) {
        return TipoMedidor::DIGITAL;
    }
    return TipoMedidor::ANALOGICO;
}