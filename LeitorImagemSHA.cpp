// LeitorImagemSHA.cpp (O ADAPTER)

#include "LeitorImagemSHA.hpp"
#include "EstrategiasOCR.hpp"

#include <filesystem>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <limits>

// =====================================================================
// STB_IMAGE IMPLEMENTATION
// (Deve existir em APENAS UM .cpp do projeto)
// =====================================================================
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// =====================================================================

namespace fs = std::filesystem;

static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return (char)std::tolower(c); });
    return s;
}

LeitorImagemSHA::LeitorImagemSHA(SHAConfigDAO* dao) : configDAO(dao) {
    Logger::getInstance()->registrarInfo("LeitorImagemSHA",
        "Adapter inicializado (resolve caminho de imagem no diretório do SHA).");
}

std::string LeitorImagemSHA::resolverCaminhoImagemMaisRecente(const std::string& diretorio) const {
    if (diretorio.empty()) return "";

    fs::path dirPath = fs::path(diretorio);

    // Se o usuário salvou "C:\...\SHA" sem barra final, o path ainda é válido.
    // Se tiver espaços etc, filesystem dá conta.
    std::error_code ec;
    if (!fs::exists(dirPath, ec) || !fs::is_directory(dirPath, ec)) {
        Logger::getInstance()->registrarErro("LeitorImagemSHA",
            "Diretório inválido/não existe: " + diretorio);
        return "";
    }

    // 1) Prioriza o arquivo fixo (quando o SHA sobrescreve sempre o mesmo)
    {
        fs::path fixedBmp = dirPath / "leitura_hidrometro.bmp";
        if (fs::exists(fixedBmp, ec) && fs::is_regular_file(fixedBmp, ec)) {
            return fixedBmp.string();
        }

        fs::path fixedPng = dirPath / "leitura_hidrometro.png";
        if (fs::exists(fixedPng, ec) && fs::is_regular_file(fixedPng, ec)) {
            return fixedPng.string();
        }
    }

    // 2) Caso contrário, pega a imagem mais recente no diretório
    const std::vector<std::string> exts = {".bmp", ".png", ".jpg", ".jpeg"};

    bool found = false;
    fs::path bestPath;
    fs::file_time_type bestTime = fs::file_time_type::min();

    for (const auto& entry : fs::directory_iterator(dirPath, ec)) {
        if (ec) break;
        if (!entry.is_regular_file(ec)) continue;

        fs::path p = entry.path();
        std::string ext = toLower(p.extension().string());

        if (std::find(exts.begin(), exts.end(), ext) == exts.end())
            continue;

        fs::file_time_type t = entry.last_write_time(ec);
        if (ec) continue;

        if (!found || t > bestTime) {
            found = true;
            bestTime = t;
            bestPath = p;
        }
    }

    if (!found) {
        Logger::getInstance()->registrarErro("LeitorImagemSHA",
            "Nenhuma imagem encontrada no diretório: " + diretorio);
        return "";
    }

    return bestPath.string();
}

Imagem LeitorImagemSHA::obterImagem(const std::string& idSHA) {
    if (!configDAO) {
        Logger::getInstance()->registrarErro("LeitorImagemSHA", "configDAO == nullptr");
        return Imagem(idSHA, "", "");
    }

    ConfiguracaoSHA config = configDAO->buscarConfiguracao(idSHA);

    // Importante: buscarConfiguracao retorna vazio se não achou (idUsuario=0, diretorio="")
    if (config.idUsuario == 0 || config.diretorio.empty()) {
        Logger::getInstance()->registrarErro("LeitorImagemSHA",
            "Configuração não encontrada para idSHA: " + idSHA);
        return Imagem(idSHA, "", "");
    }

    std::string caminho = resolverCaminhoImagemMaisRecente(config.diretorio);
    if (caminho.empty()) {
        // fallback: assume arquivo padrão
        fs::path fallback = fs::path(config.diretorio) / "leitura_hidrometro.bmp";
        caminho = fallback.string();
        Logger::getInstance()->registrarErro("LeitorImagemSHA",
            "Usando fallback (pode não existir): " + caminho);
    } else {
        Logger::getInstance()->registrarInfo("LeitorImagemSHA",
            "Imagem selecionada: " + caminho);
    }

    return Imagem(idSHA, "Buffer_Simulado", caminho);
}

TipoMedidor LeitorImagemSHA::determinarTipoMedidor(const std::string& idSHA) {
    std::string low = toLower(idSHA);

    // Heurística simples baseada no idSHA (você pode trocar por algo do DB depois)
    if (low.find("dig") != std::string::npos) return DIGITAL;
    if (low.find("ana") != std::string::npos) return ANALOGICO;

    // padrão: digital (porque teu exemplo é digital)
    return DIGITAL;
}

// --------------------------
// LEGADO (fallback por pixel)
// --------------------------

double LeitorImagemSHA::lerROIReal(const std::string& caminhoCompletoDaImagem) const {
    if (caminhoCompletoDaImagem.empty()) return 0.0;

    int largura = 0, altura = 0, canais = 0;

    // força 3 canais (RGB) pra simplificar
    unsigned char* data = stbi_load(caminhoCompletoDaImagem.c_str(), &largura, &altura, &canais, 3);

    if (!data || largura <= 0 || altura <= 0) {
        Logger::getInstance()->registrarErro("LeitorImagemSHA",
            "Falha ao carregar imagem: " + caminhoCompletoDaImagem +
            " | Motivo: " + std::string(stbi_failure_reason() ? stbi_failure_reason() : "desconhecido"));
        if (data) stbi_image_free(data);
        return 0.0;
    }

    // Ponto dentro da área dos dígitos (proporcional, funciona em 800x800 e também em outras resoluções)
    int x = (int)std::round(largura * 0.35);
    int y = (int)std::round(altura * 0.36);

    x = std::clamp(x, 0, largura - 1);
    y = std::clamp(y, 0, altura - 1);

    int idx = (y * largura + x) * 3;
    unsigned char r = data[idx + 0];
    unsigned char g = data[idx + 1];
    unsigned char b = data[idx + 2];

    // valor “escuro” (pega preto e vermelho)
    unsigned char m = (unsigned char)std::min({r, g, b});

    stbi_image_free(data);

    // Simulação determinística (legado)
    double volume_base = 100.0 + (double)m;
    double volume = std::round(volume_base * 10.0) / 10.0;

    Logger::getInstance()->registrarInfo("LeitorImagemSHA",
        "LEGADO: pixel(" + std::to_string(x) + "," + std::to_string(y) +
        ")=" + std::to_string((int)m) + " -> volume=" + std::to_string(volume) + " m3");

    return volume;
}

double LeitorImagemSHA::obterLeituraVolume(const std::string& idSHA) {
    // Mantido por compatibilidade. O ideal agora é:
    // SubsistemaDados -> obterImagem() -> Strategy OCR -> interpretar()
    Imagem img = obterImagem(idSHA);
    if (img.caminhoArquivo.empty()) {
        Logger::getInstance()->registrarErro("LeitorImagemSHA",
            "obterLeituraVolume (legado) falhou: caminhoArquivo vazio.");
        return 0.0;
    }
    return lerROIReal(img.caminhoArquivo);
}
