// EstrategiasOCR.hpp

#ifndef ESTRATEGIAS_OCR_H
#define ESTRATEGIAS_OCR_H

#include "InterpretadorConsumo.hpp"
#include "Logger.hpp"

// Só inclua o stb_image.h (SEM definir STB_IMAGE_IMPLEMENTATION aqui!)
// A implementação já está no teu LeitorImagemSHA.cpp
#include "stb_image.h"

#include <array>
#include <algorithm>
#include <cstdint>
#include <cmath>
#include <limits>
#include <string>
#include <vector>

#include <fstream>
#include <filesystem>

namespace ocr_detail {

struct BinImg {
    int w = 0;
    int h = 0;
    std::vector<uint8_t> px; // 0/1
};

static inline int clampi(int v, int lo, int hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

static inline uint8_t isInk(uint8_t r, uint8_t g, uint8_t b, uint8_t thrMin) {
    // Para pegar preto e vermelho bem: usa o menor canal (vermelho tem G/B baixos)
    uint8_t m = std::min({r, g, b});
    return (m < thrMin) ? 1u : 0u;
}

static inline std::vector<uint8_t> buildInkMaskRGB(
    const unsigned char* rgb, int w, int h, int ch, uint8_t thrMin)
{
    std::vector<uint8_t> ink((size_t)w * (size_t)h, 0);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = (y * w + x) * ch;
            uint8_t r = (ch >= 1) ? rgb[idx + 0] : 0;
            uint8_t g = (ch >= 2) ? rgb[idx + 1] : r;
            uint8_t b = (ch >= 3) ? rgb[idx + 2] : r;
            ink[(size_t)y * (size_t)w + (size_t)x] = isInk(r, g, b, thrMin);
        }
    }
    return ink;
}

static inline std::vector<int> colSums(const std::vector<uint8_t>& ink, int w, int h) {
    std::vector<int> sums(w, 0);
    for (int x = 0; x < w; ++x) {
        int s = 0;
        for (int y = 0; y < h; ++y) s += ink[(size_t)y * (size_t)w + (size_t)x];
        sums[x] = s;
    }
    return sums;
}

static inline std::vector<double> smooth1D(const std::vector<int>& v, int k) {
    if (k < 1) k = 1;
    if (k % 2 == 0) k += 1;
    int n = (int)v.size();
    int r = k / 2;
    std::vector<double> s(n, 0.0);

    for (int i = 0; i < n; ++i) {
        int a = std::max(0, i - r);
        int b = std::min(n - 1, i + r);
        int sum = 0;
        int cnt = 0;
        for (int j = a; j <= b; ++j) { sum += v[j]; ++cnt; }
        s[i] = (cnt > 0) ? (double)sum / (double)cnt : 0.0;
    }
    return s;
}

static inline std::vector<int> findSeparatorPeaks(
    const std::vector<double>& smooth,
    int roiW, int roiH,
    int expectedPeaks // 5 barras internas
) {
    std::vector<int> peaks;

    // limiar: barras são colunas com bastante "tinta"
    const double thr = std::max(10.0, 0.35 * (double)roiH);

    for (int x = 1; x < roiW - 1; ++x) {
        if (smooth[x] >= smooth[x - 1] && smooth[x] > smooth[x + 1] && smooth[x] > thr) {
            peaks.push_back(x);
        }
    }

    // remove coisas muito próximas das bordas (borda da janelinha)
    peaks.erase(
        std::remove_if(peaks.begin(), peaks.end(), [&](int x){
            return x < (int)(0.08 * roiW) || x > (int)(0.92 * roiW);
        }),
        peaks.end()
    );

    // merge de picos colados
    std::sort(peaks.begin(), peaks.end());
    std::vector<int> merged;
    for (int p : peaks) {
        if (merged.empty()) {
            merged.push_back(p);
        } else {
            int last = merged.back();
            if (std::abs(p - last) <= 8) {
                // média simples
                merged.back() = (last + p) / 2;
            } else {
                merged.push_back(p);
            }
        }
    }

    // Se deu mais que 5, pega os 5 mais "fortes"
    if ((int)merged.size() > expectedPeaks) {
        struct PeakVal { int x; double v; };
        std::vector<PeakVal> pv;
        pv.reserve(merged.size());
        for (int x : merged) pv.push_back({x, smooth[x]});

        std::sort(pv.begin(), pv.end(), [](const PeakVal& a, const PeakVal& b){
            return a.v > b.v;
        });

        pv.resize((size_t)expectedPeaks);
        merged.clear();
        for (auto& e : pv) merged.push_back(e.x);
        std::sort(merged.begin(), merged.end());
    }

    return merged;
}

static inline BinImg cropToInkAndResize(
    const std::vector<uint8_t>& ink, int w, int h,
    int x0, int x1, int y0, int y1,
    int outW, int outH
) {
    x0 = clampi(x0, 0, w - 1);
    x1 = clampi(x1, 1, w);
    y0 = clampi(y0, 0, h - 1);
    y1 = clampi(y1, 1, h);

    // bbox de tinta
    int minX = x1, minY = y1, maxX = x0, maxY = y0;
    bool found = false;

    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            if (ink[(size_t)y * (size_t)w + (size_t)x]) {
                found = true;
                minX = std::min(minX, x);
                minY = std::min(minY, y);
                maxX = std::max(maxX, x);
                maxY = std::max(maxY, y);
            }
        }
    }

    // se não achou tinta, devolve tudo zero
    if (!found) {
        BinImg out;
        out.w = outW; out.h = outH;
        out.px.assign((size_t)outW * (size_t)outH, 0);
        return out;
    }

    // padding
    const int pad = 2;
    minX = clampi(minX - pad, 0, w - 1);
    minY = clampi(minY - pad, 0, h - 1);
    maxX = clampi(maxX + pad, 0, w - 1);
    maxY = clampi(maxY + pad, 0, h - 1);

    int srcW = (maxX - minX + 1);
    int srcH = (maxY - minY + 1);

    // resize nearest-neighbor do recorte
    BinImg out;
    out.w = outW; out.h = outH;
    out.px.assign((size_t)outW * (size_t)outH, 0);

    for (int oy = 0; oy < outH; ++oy) {
        int sy = minY + (int)std::floor((double)oy * (double)srcH / (double)outH);
        sy = clampi(sy, minY, maxY);

        for (int ox = 0; ox < outW; ++ox) {
            int sx = minX + (int)std::floor((double)ox * (double)srcW / (double)outW);
            sx = clampi(sx, minX, maxX);

            out.px[(size_t)oy * (size_t)outW + (size_t)ox] = ink[(size_t)sy * (size_t)w + (size_t)sx];
        }
    }

    return out;
}

static inline int binaryDiff(const BinImg& a, const BinImg& b) {
    if (a.w != b.w || a.h != b.h) return std::numeric_limits<int>::max();
    int diff = 0;
    for (size_t i = 0; i < a.px.size(); ++i) diff += (a.px[i] != b.px[i]) ? 1 : 0;
    return diff;
}

static inline bool tryLoadTemplateDigit(
    const std::string& base, int digit,
    BinImg& out,
    int outW, int outH,
    uint8_t thrMin
) {
    // tenta .png e .bmp
    const std::array<std::string, 2> exts = {".png", ".bmp"};

    for (const auto& ext : exts) {
        std::string path = base + std::to_string(digit) + ext;

        int w = 0, h = 0, ch = 0;
        unsigned char* data = stbi_load(path.c_str(), &w, &h, &ch, 3);
        if (!data) continue;

        ch = 3;
        auto ink = buildInkMaskRGB(data, w, h, ch, thrMin);
        stbi_image_free(data);

        // recorta bbox de tinta (a imagem toda) e redimensiona
        out = cropToInkAndResize(ink, w, h, 0, w, 0, h, outW, outH);
        return true;
    }

    return false;
}

static inline bool loadTemplates(
    std::array<BinImg, 10>& tpl,
    int outW, int outH,
    uint8_t thrMin,
    std::string* usedBase = nullptr
) {
    const std::array<std::string, 4> bases = {
        "templates/digital/",
        "./templates/digital/",
        "../templates/digital/",
        "assets/templates/digital/"
    };

    for (const auto& base : bases) {
        bool ok = true;
        for (int d = 0; d <= 9; ++d) {
            if (!tryLoadTemplateDigit(base, d, tpl[(size_t)d], outW, outH, thrMin)) {
                ok = false;
                break;
            }
        }
        if (ok) {
            if (usedBase) *usedBase = base;
            return true;
        }
    }
    return false;
}

} // namespace ocr_detail

static void savePPM(const std::string& path,
                    const std::vector<unsigned char>& rgb, int w, int h) {
    std::ofstream f(path, std::ios::binary);
    f << "P6\n" << w << " " << h << "\n255\n";
    f.write(reinterpret_cast<const char*>(rgb.data()), (std::streamsize)rgb.size());
}

static void savePGM01(const std::string& path,
                      const std::vector<uint8_t>& bin01, int w, int h) {
    std::ofstream f(path, std::ios::binary);
    f << "P5\n" << w << " " << h << "\n255\n";
    for (auto v01 : bin01) {
        unsigned char v = v01 ? 0 : 255; // tinta=preto
        f.write(reinterpret_cast<const char*>(&v), 1);
    }
}


// --- Implementação para Medidores Digitais ---
class InterpretadorDigital : public InterpretadorConsumo {
public:
    double interpretar(const Imagem& imagem) override {
        Logger::getInstance()->registrarInfo("Strategy:Digital", "OCR digital: iniciando leitura em: " + imagem.caminhoArquivo);

        if (imagem.caminhoArquivo.empty()) {
            Logger::getInstance()->registrarErro("Strategy:Digital", "caminhoArquivo vazio.");
            return 0.0;
        }

        int imgW = 0, imgH = 0, ch = 0;
        unsigned char* data = stbi_load(imagem.caminhoArquivo.c_str(), &imgW, &imgH, &ch, 3);
        if (!data) {
            Logger::getInstance()->registrarErro("Strategy:Digital", "Falha ao abrir imagem via stb_image.");
            return 0.0;
        }
        ch = 3;

        // ROI calibrada pro teu SHA (800x800) e escalável por proporção
        // (valores base: x=225..545, y=250..340)
        const int x0 = (int)std::round(imgW * 0.28125);
        const int x1 = (int)std::round(imgW * 0.68125);
        const int y0 = (int)std::round(imgH * 0.31250);
        const int y1 = (int)std::round(imgH * 0.42500);

        const int roiX0 = ocr_detail::clampi(x0, 0, imgW - 1);
        const int roiX1 = ocr_detail::clampi(x1, 1, imgW);
        const int roiY0 = ocr_detail::clampi(y0, 0, imgH - 1);
        const int roiY1 = ocr_detail::clampi(y1, 1, imgH);

        const int roiW = roiX1 - roiX0;
        const int roiH = roiY1 - roiY0;

        if (roiW < 50 || roiH < 30) {
            stbi_image_free(data);
            Logger::getInstance()->registrarErro("Strategy:Digital", "ROI muito pequena/inválida.");
            return 0.0;
        }

        // Recorta ROI (RGB)
        std::vector<unsigned char> roiRGB((size_t)roiW * (size_t)roiH * 3);
        std::filesystem::create_directories("debug_ocr");
        savePPM("debug_ocr/roi.ppm", roiRGB, roiW, roiH);
        for (int y = 0; y < roiH; ++y) {
            for (int x = 0; x < roiW; ++x) {
                int sx = roiX0 + x;
                int sy = roiY0 + y;
                int srcIdx = (sy * imgW + sx) * 3;
                int dstIdx = (y * roiW + x) * 3;
                roiRGB[(size_t)dstIdx + 0] = data[srcIdx + 0];
                roiRGB[(size_t)dstIdx + 1] = data[srcIdx + 1];
                roiRGB[(size_t)dstIdx + 2] = data[srcIdx + 2];
            }
        }
        stbi_image_free(data);

        // Binarização (pega preto + vermelho)
        const uint8_t thrMin = 160; // pode ajustar se precisar
        auto ink = ocr_detail::buildInkMaskRGB(roiRGB.data(), roiW, roiH, 3, thrMin);

        // Detecta separadores verticais
        auto cs = ocr_detail::colSums(ink, roiW, roiH);
        auto sm = ocr_detail::smooth1D(cs, 7);
        auto seps = ocr_detail::findSeparatorPeaks(sm, roiW, roiH, 5);

        if ((int)seps.size() != 5) {
            Logger::getInstance()->registrarErro("Strategy:Digital",
                "Não consegui detectar 5 separadores. Vou cair no split fixo (pode errar).");
        }

        // Carrega templates (cache)
        constexpr int DIG_W = 19;
        constexpr int DIG_H = 35;

        static bool templatesLoaded = false;
        static std::array<ocr_detail::BinImg, 10> templates;
        static std::string baseUsed;

        if (!templatesLoaded) {
            if (!ocr_detail::loadTemplates(templates, DIG_W, DIG_H, thrMin, &baseUsed)) {
                Logger::getInstance()->registrarErro("Strategy:Digital",
                    "Templates 0..9 não encontrados em templates/digital/. Crie: templates/digital/0.png ... 9.png");
                return 0.0;
            }
            templatesLoaded = true;
            Logger::getInstance()->registrarInfo("Strategy:Digital", "Templates carregados de: " + baseUsed);
        }

        // Define caixas dos 6 dígitos
        std::array<int, 7> cuts{}; // [0]=inicio, [1..5]=seps, [6]=fim
        if ((int)seps.size() == 5) {
            cuts[0] = 0;
            for (int i = 0; i < 5; ++i) cuts[i + 1] = seps[(size_t)i];
            // tenta achar “borda direita” pelo último x com tinta razoável
            int right = roiW - 1;
            int thrRight = std::max(6, (int)std::round(0.30 * roiH));
            for (int x = roiW - 1; x >= 0; --x) {
                if (cs[(size_t)x] >= thrRight) { right = x; break; }
            }
            cuts[6] = right;
        } else {
            // fallback: divide em 6 partes iguais
            cuts[0] = 0;
            for (int i = 1; i <= 6; ++i) cuts[i] = (int)std::round((double)roiW * (double)i / 6.0);
        }

        const int marginX = 3;
        std::array<int, 6> digits{};

        for (int i = 0; i < 6; ++i) {
            int xL = cuts[i] + marginX;
            int xR = cuts[i + 1] - marginX;
            // ===== AJUSTE FINO HORIZONTAL POR DÍGITO =====
            // (+) empurra pra direita, (-) empurra pra esquerda
            static const int xOffsetL[6] = {
                60,  // d0 (corta um pouco mais da esquerda)
                5,  // d1
                5,  // d2
                5,  // d3
                5,  // d4
                5  // d5 (abre mais pra esquerda)
            };

            static const int xOffsetR[6] = {
                -3,  // d0
                0,  // d1
                0,  // d2
                0,  // d3
                0,  // d4
                -1   // d5 (corta menos à direita / pega mais)
            };

            xL += xOffsetL[i];
            xR += xOffsetR[i];

            // segurança
            xL = ocr_detail::clampi(xL, 0, roiW - 1);
            xR = ocr_detail::clampi(xR, xL + 1, roiW);
            if (xR <= xL) { xL = cuts[i]; xR = cuts[i + 1]; }

            // ===== AJUSTES FINOS POR DÍGITO =====
            // valores em pixels (ajuste olhando os .pgm)
            static const int yOffsetTop[6] = {
                64,  // d0  -> desce mais (borda esquerda)
                65,  // d1
                60,  // d2
                60,  // d3
                60,  // d4
                58   // d5  -> desce mais (borda direita)
            };

            static const int yOffsetBot[6] = {
                -6,  // d0
                2,  // d1
                -2,  // d2
                0,  // d3
                2,  // d4
                2   // d5
            };

            // base
            int yTop = 0;
            int yBot = roiH;

            // aplica zoom vertical específico
            yTop += yOffsetTop[i];
            yBot -= yOffsetBot[i];

            // segurança
            yTop = ocr_detail::clampi(yTop, 0, roiH - 1);
            yBot = ocr_detail::clampi(yBot, yTop + 1, roiH);


            auto bin = ocr_detail::cropToInkAndResize(ink, roiW, roiH, xL, xR, yTop, yBot, DIG_W, DIG_H);
            savePGM01("debug_ocr/d" + std::to_string(i) + ".pgm", bin.px, bin.w, bin.h);
            // template matching
            int bestD = 0;
            int bestScore = std::numeric_limits<int>::max();

            for (int d = 0; d <= 9; ++d) {
                int score = ocr_detail::binaryDiff(bin, templates[(size_t)d]);
                if (score < bestScore) {
                    bestScore = score;
                    bestD = d;
                }
            }
            digits[(size_t)i] = bestD;
        }

        // Monta leitura: D0D1D2D3.D4D5  (m³)
        int inteiro = digits[0] * 1000 + digits[1] * 100 + digits[2] * 10 + digits[3];
        int dec2 = digits[4] * 10 + digits[5];
        double leitura_m3 = (double)inteiro + ((double)dec2 / 100.0);

        // Log bonitinho
        std::string s =
            std::to_string(digits[0]) + std::to_string(digits[1]) +
            std::to_string(digits[2]) + std::to_string(digits[3]) +
            "." + std::to_string(digits[4]) + std::to_string(digits[5]);

        Logger::getInstance()->registrarInfo("Strategy:Digital", "Leitura extraída (m3): " + s);

        return leitura_m3;
    }
};


// --- Implementação para Medidores Analógicos ---
class InterpretadorAnalogico : public InterpretadorConsumo {
private:
    std::string modeloOCR;

public:
    InterpretadorAnalogico() : modeloOCR("Modelo P1.0") {}

    double interpretar(const Imagem& imagem) override {
        Logger::getInstance()->registrarInfo("Strategy:Analogico",
            "OCR analógico ainda não implementado. Modelo: " + modeloOCR);
        return 0.0;
    }
};

#endif // ESTRATEGIAS_OCR_H
