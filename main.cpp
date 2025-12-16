// main.cpp - VERSÃO CLI INTERATIVA (CORRIGIDO)

#include "MonitoramentoFacade.hpp"
#include "DBConnection.hpp"
#include "UsuarioDAOImpl.hpp"
#include "SubsistemaUsuarios.hpp"
#include "NotificacaoFactory.hpp"
#include "SubsistemaAlerta.hpp"
#include "LeitorImagemSHA.hpp"
#include "ConsumoHistoricoDAO.hpp" 
#include "SubsistemaDados.hpp"
#include "SHAConfigDAO.hpp"
#include "Entidades.hpp" // Para Usuario, ConfiguracaoSHA, ConsumoDTO, etc.
#include "Logger.hpp" // Para registrarInfo/Erro
#include <iostream>
#include <limits>
#include <string>
#include "sqlite3.h"

// Funções para a Interface de Usuário
void exibirMenu() {
   std::cout << "\n=============================================" << std::endl;
   std::cout << "          PAINEL DE MONITORAMENTO (CLI)" << std::endl;
   std::cout << "=============================================" << std::endl;
   std::cout << "1. Criar Novo Usuario (Nome e CPF)" << std::endl;
   std::cout << "2. Definir Limite de Alerta para um ID" << std::endl;
   std::cout << "3. PROCESSAR LEITURA (ID do Usuario e SHA)" << std::endl;
   std::cout << "4. Simular Consulta Consolidada (RF 2.3)" << std::endl;
   std::cout << "5. VISUALIZAR DADOS (Usuarios e SHAs)" << std::endl; 
   std::cout << "6. CONFIGURAR NOVO SHA (Diretorio e Usuario)" << std::endl; 
   std::cout << "0. Sair" << std::endl;
   std::cout << "---------------------------------------------" << std::endl;
   std::cout << "Digite sua opcao: ";
}

// ... (Funções callback e visualizarDadosBrutos - Não alteradas, assumidas como corretas) ...

// Função de Callback para exibir os resultados da query
static int callback(void *data, int argc, char **argv, char **azColName) {
   for (int i = 0; i < argc; i++) {
      std::cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << " | ";
   }
   std::cout << "\n";
   return 0;
}

// Função de acesso direto ao DB
void visualizarDadosBrutos(const char* dbPath) {
   sqlite3 *db;
   char *zErrMsg = 0;
   int rc;

   rc = sqlite3_open(dbPath, &db);

   if (rc != SQLITE_OK) {
      std::cerr << "Erro ao abrir DB: " << sqlite3_errmsg(db) << std::endl;
      return;
   }
    // ... (Lógica de exibição de tabelas) ...

   std::cout << "\n--- DADOS BRUTOS DO BANCO ---" << std::endl;
   
   std::cout << "\n[Tabela USUARIOS]" << std::endl;
   const char* sql_users = "SELECT idUsuario, nome, cpf FROM Usuarios;";
   rc = sqlite3_exec(db, sql_users, callback, 0, &zErrMsg);
   if (rc != SQLITE_OK) {
      std::cerr << "SQL erro (USUARIOS): " << zErrMsg << std::endl;
      sqlite3_free(zErrMsg);
   }
   
   std::cout << "\n[Tabela HISTORICO (Leituras)]" << std::endl;
   const char* sql_historico = "SELECT idSHA, volume, dataHora FROM Historico;";
   rc = sqlite3_exec(db, sql_historico, callback, 0, &zErrMsg);
   if (rc != SQLITE_OK) {
      std::cerr << "SQL erro (HISTORICO): " << zErrMsg << std::endl;
      sqlite3_free(zErrMsg);
   }

    std::cout << "\n[Tabela ConfiguracaoSHA]" << std::endl;
   const char* sql_config = "SELECT idSHA, idUsuario, diretorio FROM ConfiguracaoSHA;";
   rc = sqlite3_exec(db, sql_config, callback, 0, &zErrMsg);
   if (rc != SQLITE_OK) {
      std::cerr << "SQL erro (ConfiguracaoSHA): " << zErrMsg << std::endl;
      sqlite3_free(zErrMsg);
   }
   
   sqlite3_close(db);
   std::cout << "-----------------------------------------------" << std::endl;
}


void processarComando(int comando, MonitoramentoFacade& fachada, SHAConfigDAO& shaConfigDAO, UsuarioDAOImpl& uDao) {
   int idUsuario;
   std::string nome, cpf, idSHA;
   double limite;

   auto limparBuffer = []() {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
   };

   switch (comando) {
      case 1: { // Criar Novo Usuário
         // ... (Lógica de input e criação do usuário, utilizando fachada.criarUsuario) ...
            std::cout << "--- Criar Novo Usuário ---" << std::endl;
         std::cout << "-> ID do novo usuario (ex: 102): ";
         if (!(std::cin >> idUsuario)) { limparBuffer(); std::cout << "ID inválido." << std::endl; break; }
         std::cout << "-> Nome do usuario: ";
         std::cin >> std::ws; 
         std::getline(std::cin, nome);
         std::cout << "-> CPF (sem formatacao): ";
         std::cin >> cpf;
         
         Usuario novoUser = {idUsuario, nome, cpf, {}};
         if (fachada.criarUsuario(novoUser)) {
            std::cout << "\n✅ SUCESSO! Usuário " << nome << " (ID: " << idUsuario << ") registrado no DB." << std::endl;
         } else {
            std::cout << "\n❌ FALHA! Não foi possível criar o usuário." << std::endl;
         }
         break;
      }
      case 2: { // Definir Limite de Alerta (Por SHA)
         // ... (Lógica de input e definição de limite, utilizando fachada.definirLimiteAlerta) ...
            std::cout << "--- Definir Limite de Alerta (Por SHA) ---" << std::endl;
         std::cout << "-> ID do usuario proprietário: ";
         if (!(std::cin >> idUsuario)) { limparBuffer(); std::cout << "ID inválido." << std::endl; break; }
         
         std::cout << "-> ID do SHA (ex: SHA-DIG-456): "; 
         std::cin >> idSHA; 
         
         std::cout << "-> Limite de consumo (m3, ex: 80.0): ";
         if (!(std::cin >> limite)) { limparBuffer(); std::cout << "Limite inválido." << std::endl; break; }

         if (fachada.definirLimiteAlerta(idUsuario, idSHA, limite)) { 
            std::cout << "\n✅ SUCESSO! Limite de " << limite << " m³ definido para o SHA " << idSHA << "." << std::endl;
         } else {
            std::cout << "\n❌ FALHA! Nao foi possivel definir o limite (Usuario ou SHA Invalido)." << std::endl;
         }
         break;
      }
      case 3: { // PROCESSAR LEITURA (Template Method, Adapter)
         // ... (Lógica de processamento, utilizando fachada.processarLeituraDiaria) ...
            std::cout << "--- Processar Leitura (Template Method) ---" << std::endl;
         std::cout << "-> ID do usuario para monitorar: ";
         if (!(std::cin >> idUsuario)) { limparBuffer(); std::cout << "ID inválido." << std::endl; break; }
         std::cout << "-> ID do SHA (ex: SHA-DIG-456): ";
         std::cin >> idSHA;
         
         std::cout << "\n--- EXECUTANDO FLUXO FIXO (Template Method) ---" << std::endl;
         fachada.processarLeituraDiaria(idSHA, idUsuario); 
         
         std::cout << "\n✅ FLUXO CONCLUÍDO! Verifique o Logger acima." << std::endl;
         break;
      }
      case 4: { // Simular Consulta Consolidada
         // ... (Lógica de consulta, utilizando fachada.monitorarConsumoUsuario) ...
            std::cout << "--- Simular Consulta Consolidada ---" << std::endl;
         std::cout << "-> ID do usuario para consulta: ";
         if (!(std::cin >> idUsuario)) { limparBuffer(); std::cout << "ID inválido." << std::endl; break; }
         
         ConsumoDTO c = fachada.monitorarConsumoUsuario(idUsuario, 0, 0); 
         
         std::cout << "\n RESULTADO (DAO Agregação): Consumo Total para o ID " << idUsuario 
                  << ": " << c.totalConsumido << " m³." << std::endl;
         break;
      }

      case 5: { // VISUALIZAR DADOS (TABELAS)
         // Uso da lógica de exibição de dados da CLI
         visualizarDadosBrutos("monitoramento.db"); 
            
            // Lógica de visualização de usuários e SHAs ativos (cruzamento de dados)
            std::cout << "\n=============================================" << std::endl;
         std::cout << "          Tabela: Usuarios e SHAs Ativos " << std::endl;
         std::cout << "=============================================" << std::endl;
         
         // Note: uDao e shaConfigDAO devem ser usados aqui, como no código original.
         // 1. Obter todos os usuários
         std::vector<Usuario> todosUsuarios = uDao.listarTodos(); 
         
         // 2. Obter todas as configurações de SHA
         std::vector<ConfiguracaoSHA> todasConfiguracoesSHA = shaConfigDAO.listarTodosAtivos();
         
         for (const auto& user : todosUsuarios) {
            std::cout << "ID Usuario: " << user.idUsuario 
                  << " | Nome: " << user.nome 
                  << " | CPF: " << user.cpf;
            
            bool temSHA = false;
            
            for (const auto& config : todasConfiguracoesSHA) {
               if (config.idUsuario == user.idUsuario) {
                  if (!temSHA) { std::cout << "\n\t--> SHAs Ativos: "; temSHA = true; } else { std::cout << ", "; }
                  std::cout << config.idSHA << " [Dir: " << config.diretorio << "]";
               }
            }
            
            if (!temSHA) {
               std::cout << "\n\t--> SHAs Ativos: NENHUM CADASTRADO";
            }
            std::cout << "\n---------------------------------------------" << std::endl;
         }
         
         break;
      }

      case 6: { // CONFIGURAR NOVO SHA (Vínculo)
         // ... (Lógica de input e salvamento, utilizando shaConfigDAO.salvarConfiguracao) ...
            std::string diretorio;
         
         std::cout << "--- Configurar Novo SHA (Vínculo Lógico/Físico) ---" << std::endl;
         std::cout << "-> ID do Usuario Proprietário: ";
         if (!(std::cin >> idUsuario)) { limparBuffer(); std::cout << "ID inválido." << std::endl; break; }
         std::cout << "-> ID Lógico do SHA (ex: SHA-DIG-456): ";
         std::cin >> idSHA;
         std::cout << "-> Diretório Físico (Ex: C:/Users/.../SHAs/SHA-DIG-456/): ";
         std::cin >> std::ws;
         std::getline(std::cin, diretorio);
         
         ConfiguracaoSHA config(idSHA, idUsuario, diretorio); 
         
         if (shaConfigDAO.salvarConfiguracao(config)) {
            std::cout << "\n✅ SUCESSO! SHA " << idSHA << " configurado e vinculado ao diretório (DAO)." << std::endl;
         } else {
            std::cout << "\n❌ FALHA! Nao foi possivel salvar a configuracao. (Erro de DB)." << std::endl;
         }
         break;
      }
      case 0:
         std::cout << "\nEncerrando sistema. Adeus!" << std::endl;
         break;
      default:
         std::cout << "Opção inválida. Tente novamente." << std::endl;
   }
}


void inicializarSistema() {
   // --- 1. Inicializa dependências (DAOs) ---
   DBConnection conn;
   
   ConsumoHistoricoDAO cDao(&conn);
   LimiteAlertaDAO lDao(&conn); 
   SHAConfigDAO shaConfigDAO(&conn); 
   
   // CORREÇÃO 1: Consolidação do DAO de Usuário (uDaoImpl será a única instância)
   UsuarioDAOImpl uDaoImpl(&conn);
   
   // CORREÇÃO 2: Passar a referência da instância única (uDao é apenas uma cópia local que será passada para processarComando)
   UsuarioDAOImpl uDao = uDaoImpl; 
   
   // 2. Inicializa Subsistemas (Injeção de Dependência)
   SubsistemaUsuarios subsUsuarios(&uDaoImpl);
   NotificadorFactory notifFactory;
   SubsistemaAlerta subsAlerta(&notifFactory, &lDao);
   
   // CORREÇÃO 3: Inicialização do Adapter
   LeitorImagemSHA leitor(&shaConfigDAO);
   
   // CORREÇÃO 4: Inicialização do SubsistemaDados (4 argumentos obrigatórios)
   SubsistemaDados subsDados(&leitor, &subsAlerta, &cDao, &shaConfigDAO); 
   
   // 3. Cria a Fachada
   MonitoramentoFacade fachada(&subsUsuarios, &subsAlerta, &subsDados);
   
   Logger::getInstance()->registrarInfo("MAIN", "Arquitetura completa e funcional. CLI ativada.");

   // --- 4. Loop Interativo ---
   int comando = -1;
   while (comando != 0) {
      exibirMenu();
      if (!(std::cin >> comando)) {
         std::cin.clear();
         std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
         comando = -1;
      }
      if (comando != 0) {
         // Passamos a fachada e os DAOs de utilidade para a CLI
         processarComando(comando, fachada, shaConfigDAO, uDaoImpl); // CORREÇÃO 5: uDaoImpl deve ser passado
      }
   }
}

int main() {
   inicializarSistema();
   return 0;
}