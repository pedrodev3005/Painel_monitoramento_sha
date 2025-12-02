// Entidades.hpp

#ifndef ENTIDADES_H
#define ENTIDADES_H

#include <string>
#include <vector>
#include <ctime>

// --- ENUMERAÇÕES ---

// Para o Subsistema de Alerta (usado no AlertaConsumo)
enum CanalAlerta { CAGEPA, USUARIO, EXTERNO };
enum TipoMedidor { ANALOGICO, DIGITAL }; // Para o Subsistema de Dados

// --- ESTRUTURAS DE DADOS (DTOs) ---

// Usado para o Subsistema de Alerta
struct AlertaConsumo {
    int idUsuario;
    std::string idHidrometro;
    double volumeExcedido;
    std::string mensagem;
    CanalAlerta destino;
};

// Usado para o Subsistema de Dados
struct LeituraConsumo {
    std::string idSHA;
    double volume;
    std::time_t dataHora;
};

// Usado para o Subsistema de Usuários
struct Conta {
    std::string idSHA; // Identificador do hidrômetro
    int idUsuario;
};

// Usado para o Subsistema de Usuários
struct Usuario {
    int idUsuario = 0;
    std::string nome;
    std::string cpf;
    std::vector<Conta> contas; // SHAs vinculados a este usuário
};

#endif // ENTIDADES_H