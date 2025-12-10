// InterpretadorConsumo.hpp

#ifndef INTERPRETADOR_CONSUMO_H
#define INTERPRETADOR_CONSUMO_H

#include "Entidades.hpp" // Inclui TipoMedidor
#include <string>


/**
 * @brief Interface (Contrato) do Padrão Strategy para algoritmos de interpretação de imagem (OCR).
 */
class InterpretadorConsumo {
public:
    virtual ~InterpretadorConsumo() = default;

    /**
     * @brief Executa o algoritmo de OCR/Visão Computacional na imagem.
     * @param imagem O arquivo de imagem a ser processado.
     * @return O volume de consumo extraído (em m3).
     */
    virtual double interpretar(const Imagem& imagem) = 0;
};

#endif // INTERPRETADOR_CONSUMO_H