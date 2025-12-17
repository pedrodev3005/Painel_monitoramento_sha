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

struct ConsumoDTO {
    double totalConsumido = 0.0;
};

// Usado para o Subsistema de Alerta
struct AlertaConsumo {
    int idUsuario;
    std::string idHidrometro;
    double volumeExcedido;
    std::string mensagem;
    CanalAlerta destino;
    int idSHA;
};

// Usado para o Subsistema de Dados
struct LeituraConsumo {
    std::string idSHA;
    double volume;
    std::time_t timestamp; 
};

// Usado para o Subsistema de Usuários
struct Conta {
    std::string idSHA; // Identificador do hidrômetro
    int idUsuario;
};

// Usado para o Subsistema de Usuários
struct Usuario {
    int idUsuario;
    std::string nome;
    std::string cpf;
    
    std::vector<std::string> shasMonitorados; // <-- CORREÇÃO: Membro que armazena os SHAs/Contas

    // --- CONSTRUTORES ---
    // Construtor do DAO (para SELECTS)
    Usuario(int id, const char* n, const char* c) 
        : idUsuario(id), 
          nome(n ? n : ""), 
          cpf(c ? c : "") 
    {}
    
    // Construtor completo/padrão (Mantenha todos)
    Usuario() : idUsuario(0) {} 

    Usuario(int id, std::string n, std::string c, std::vector<std::string> shas) 
        : idUsuario(id), nome(std::move(n)), cpf(std::move(c)), shasMonitorados(std::move(shas)) 
    {}
};

struct Imagem {
    std::string idSHA;
    std::string buffer;         // se vc usa isso ainda
    std::string caminhoArquivo; // vc usa isso no OCR

    Imagem() : idSHA(""), buffer(""), caminhoArquivo("") {} // ✅ NOVO

    Imagem(std::string sha, std::string buf, std::string caminho)
        : idSHA(std::move(sha)),
          buffer(std::move(buf)),
          caminhoArquivo(std::move(caminho)) {}
};

/**
 * @brief Entidade para armazenar o vinculo entre o ID Lógico do SHA,
 * o Usuário proprietário e o Caminho Físico do diretório de imagens.
 */
struct ConfiguracaoSHA {
    std::string idSHA;
    int idUsuario;
    std::string diretorio;

    ConfiguracaoSHA(std::string sha = "", int usuario = 0, std::string dir = "")
        : idSHA(sha), idUsuario(usuario), diretorio(dir) {}
};

#endif // ENTIDADES_H