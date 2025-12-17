// main.cpp - CLI INTERATIVA + OBSERVER + AUTO MONITORAMENTO (POR SHA SELECIONADO)

#include "MonitoramentoFacade.hpp"
#include "DBConnection.hpp"
#include "UsuarioDAOImpl.hpp"
#include "SubsistemaUsuarios.hpp"
#include "SubsistemaAlerta.hpp"
#include "LeitorImagemSHA.hpp"
#include "ConsumoHistoricoDAO.hpp"
#include "SubsistemaDados.hpp"
#include "SHAConfigDAO.hpp"
#include "Entidades.hpp"
#include "Logger.hpp"

#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <csignal>
#include <algorithm>

#include "sqlite3.h"

// ========================
// Menu
// ========================
void exibirMenu() {
   std::cout << "\n=============================================\n";
   std::cout << "          PAINEL DE MONITORAMENTO (CLI)\n";
   std::cout << "=============================================\n";
   std::cout << "1. Criar Novo Usuario (Nome e CPF)\n";
   std::cout << "2. Definir Limite de Alerta para um ID\n";
   std::cout << "3. PROCESSAR LEITURA (ID do Usuario e SHA)\n";
   std::cout << "4. Simular Consulta Consolidada (RF 2.3)\n";
   std::cout << "5. VISUALIZAR DADOS (Usuarios e SHAs)\n";
   std::cout << "6. CONFIGURAR NOVO SHA (Diretorio e Usuario)\n";
   std::cout << "7. Monitoramento Automatico (Adicionar/Remover/Parar/Listar)\n";
   std::cout << "8. Ajustar Intervalo do Monitoramento (segundos)\n";
   std::cout << "0. Sair\n";
   std::cout << "---------------------------------------------\n";
   std::cout << "Digite sua opcao: ";
}

// ========================
// DB helper (visualização bruta)
// ========================
static int callback(void*, int argc, char** argv, char** azColName) {
   for (int i = 0; i < argc; i++) {
      std::cout << azColName[i] << ": " << (argv[i] ? argv[i] : "NULL") << " | ";
   }
   std::cout << "\n";
   return 0;
}

void visualizarDadosBrutos(const char* dbPath) {
   sqlite3* db;
   char* zErrMsg = 0;
   int rc = sqlite3_open(dbPath, &db);

   if (rc != SQLITE_OK) {
      std::cerr << "Erro ao abrir DB: " << sqlite3_errmsg(db) << "\n";
      return;
   }

   std::cout << "\n--- DADOS BRUTOS DO BANCO ---\n";

   std::cout << "\n[Tabela USUARIOS]\n";
   rc = sqlite3_exec(db, "SELECT idUsuario, nome, cpf FROM Usuarios;", callback, 0, &zErrMsg);
   if (rc != SQLITE_OK) { std::cerr << "SQL erro (USUARIOS): " << zErrMsg << "\n"; sqlite3_free(zErrMsg); }

   std::cout << "\n[Tabela HISTORICO (Leituras)]\n";
   rc = sqlite3_exec(db, "SELECT idSHA, volume, dataHora FROM Historico;", callback, 0, &zErrMsg);
   if (rc != SQLITE_OK) { std::cerr << "SQL erro (HISTORICO): " << zErrMsg << "\n"; sqlite3_free(zErrMsg); }

   std::cout << "\n[Tabela ConfiguracaoSHA]\n";
   rc = sqlite3_exec(db, "SELECT idSHA, idUsuario, diretorio FROM ConfiguracaoSHA;", callback, 0, &zErrMsg);
   if (rc != SQLITE_OK) { std::cerr << "SQL erro (ConfiguracaoSHA): " << zErrMsg << "\n"; sqlite3_free(zErrMsg); }

   sqlite3_close(db);
   std::cout << "-----------------------------------------------\n";
}

// ========================
// Observer do Painel
// ========================
class PainelObserverCLI : public IObservadorAlerta {
public:
   void notificarLimiteExcedido(int idUsuario,
                               const std::string& idSHA,
                               double volumeLidoM3,
                               double limiteM3) override {
      std::lock_guard<std::mutex> lock(mu_);
      std::cout << "\n\n=================  ALERTA AUTOMATICO  =================\n";
      std::cout << "SHA: " << idSHA << "\n";
      std::cout << "Usuario: " << idUsuario << "\n";
      std::cout << "Leitura: " << volumeLidoM3 << " m3\n";
      std::cout << "Limite:  " << limiteM3 << " m3\n";
      std::cout << "STATUS: LIMITE EXCEDIDO!\n";
      std::cout << "===========================================================\n\n";
   }
private:
   std::mutex mu_;
};

// ========================
// Auto monitoramento (lista de SHAs selecionados)
// ========================
struct MonitorItem {
   std::string idSHA;
   int idUsuario;
};

static std::atomic<bool> g_rodando{true};
static std::atomic<bool> g_autoAtivo{false};
static std::atomic<int>  g_intervaloSeg{5};

static std::mutex g_muLista;
static std::vector<MonitorItem> g_listaMonitor;

// SIGINT
static void handleSigint(int) {
   g_rodando = false;
}

// helpers lista
static bool listaContemSHA(const std::string& idSHA) {
   for (const auto& it : g_listaMonitor) {
      if (it.idSHA == idSHA) return true;
   }
   return false;
}

static void listarMonitorados() {
   std::lock_guard<std::mutex> lk(g_muLista);
   std::cout << "\n--- SHAs EM MONITORAMENTO AUTOMATICO ---\n";
   if (g_listaMonitor.empty()) {
      std::cout << "(nenhum)\n";
      return;
   }
   for (const auto& it : g_listaMonitor) {
      std::cout << "SHA: " << it.idSHA << " | Usuario: " << it.idUsuario << "\n";
   }
}

static void loopAuto(MonitoramentoFacade& fachada) {
   // ✅ NÃO imprimir INFO/ERROR/CRITICAL no terminal nessa thread (mas salva no arquivo)
   Logger::getInstance()->setConsoleMinNivelThread(Logger::Nivel::ALERTA);

   while (g_rodando) {
      if (!g_autoAtivo.load()) {
         std::this_thread::sleep_for(std::chrono::milliseconds(200));
         continue;
      }

      // copia lista para evitar segurar lock durante OCR/DB
      std::vector<MonitorItem> copia;
      {
         std::lock_guard<std::mutex> lk(g_muLista);
         copia = g_listaMonitor;
      }

      // se ficou vazio, desliga
      if (copia.empty()) {
         g_autoAtivo = false;
         continue;
      }

      // processa cada SHA selecionado
      for (const auto& it : copia) {
         if (!g_rodando.load() || !g_autoAtivo.load()) break;
         fachada.processarLeituraDiaria(it.idSHA, it.idUsuario);
      }

      int s = g_intervaloSeg.load();
      if (s < 1) s = 1;
      for (int i = 0; i < s && g_rodando.load() && g_autoAtivo.load(); ++i) {
         std::this_thread::sleep_for(std::chrono::seconds(1));
      }
   }
}

// ========================
// CLI comandos
// ========================
static void processarComando(int comando,
                            MonitoramentoFacade& fachada,
                            SHAConfigDAO& shaConfigDAO,
                            UsuarioDAOImpl& uDao) {
   int idUsuario;
   std::string nome, cpf, idSHA;
   double limite;

   auto limparBuffer = []() {
      std::cin.clear();
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
   };

   switch (comando) {
      case 1: {
         std::cout << "--- Criar Novo Usuário ---\n";
         std::cout << "-> ID do novo usuario (ex: 102): ";
         if (!(std::cin >> idUsuario)) { limparBuffer(); std::cout << "ID inválido.\n"; break; }

         std::cout << "-> Nome do usuario: ";
         std::cin >> std::ws; std::getline(std::cin, nome);

         std::cout << "-> CPF (sem formatacao): ";
         std::cin >> cpf;

         Usuario novoUser = {idUsuario, nome, cpf, {}};
         if (fachada.criarUsuario(novoUser)) std::cout << "✅ Usuário criado.\n";
         else std::cout << "❌ Falha ao criar usuário.\n";
         break;
      }

      case 2: {
         std::cout << "--- Definir Limite (Por SHA) ---\n";
         std::cout << "-> ID do usuario proprietário: ";
         if (!(std::cin >> idUsuario)) { limparBuffer(); std::cout << "ID inválido.\n"; break; }

         std::cout << "-> ID do SHA (ex: 2): ";
         std::cin >> idSHA;

         std::cout << "-> Limite (m3): ";
         if (!(std::cin >> limite)) { limparBuffer(); std::cout << "Limite inválido.\n"; break; }

         if (fachada.definirLimiteAlerta(idUsuario, idSHA, limite)) std::cout << "✅ Limite definido.\n";
         else std::cout << "❌ Falha ao definir limite.\n";
         break;
      }

      case 3: {
         std::cout << "--- Processar Leitura (Manual) ---\n";
         std::cout << "-> ID do usuario: ";
         if (!(std::cin >> idUsuario)) { limparBuffer(); std::cout << "ID inválido.\n"; break; }

         std::cout << "-> ID do SHA: ";
         std::cin >> idSHA;

         std::cout << "\n--- EXECUTANDO FLUXO FIXO (Template Method) ---\n";
         fachada.processarLeituraDiaria(idSHA, idUsuario);
         std::cout << "✅ Fluxo concluído.\n";
         break;
      }

      case 4: {
         std::cout << "--- Consulta Consolidada ---\n";
         std::cout << "-> ID do usuario: ";
         if (!(std::cin >> idUsuario)) { limparBuffer(); std::cout << "ID inválido.\n"; break; }

         ConsumoDTO c = fachada.monitorarConsumoUsuario(idUsuario, 0, 0);
         std::cout << "Total: " << c.totalConsumido << " m3\n";
         break;
      }

      case 5: {
         visualizarDadosBrutos("monitoramento.db");

         std::cout << "\n==== Usuarios e SHAs Ativos ====\n";
         auto usuarios = uDao.listarTodos();
         auto shas = shaConfigDAO.listarTodosAtivos();

         for (const auto& u : usuarios) {
            std::cout << "Usuario " << u.idUsuario << " | " << u.nome << " | " << u.cpf << "\n";
            bool tem = false;
            for (const auto& c : shas) {
               if (c.idUsuario == u.idUsuario) {
                  tem = true;
                  std::cout << "   -> SHA: " << c.idSHA << " | Dir: " << c.diretorio << "\n";
               }
            }
            if (!tem) std::cout << "   -> Nenhum SHA\n";
            std::cout << "-----------------------------\n";
         }

         listarMonitorados();
         break;
      }

      case 6: {
         std::string diretorio;
         std::cout << "--- Configurar Novo SHA ---\n";
         std::cout << "-> ID do usuario: ";
         if (!(std::cin >> idUsuario)) { limparBuffer(); std::cout << "ID inválido.\n"; break; }

         std::cout << "-> ID do SHA: ";
         std::cin >> idSHA;

         std::cout << "-> Diretório: ";
         std::cin >> std::ws; std::getline(std::cin, diretorio);

         ConfiguracaoSHA cfg(idSHA, idUsuario, diretorio);
         if (shaConfigDAO.salvarConfiguracao(cfg)) std::cout << "✅ SHA configurado.\n";
         else std::cout << "❌ Falha ao salvar config.\n";
         break;
      }

      case 7: {
         int op = 0;
         std::cout << "\n--- Monitoramento Automatico ---\n";
         std::cout << "1) Adicionar/Ativar um SHA\n";
         std::cout << "2) Remover/Desativar um SHA\n";
         std::cout << "3) Parar TODOS\n";
         std::cout << "4) Listar SHAs monitorados\n";
         std::cout << "Opcao: ";
         if (!(std::cin >> op)) { limparBuffer(); std::cout << "Opção inválida.\n"; break; }

         if (op == 1) {
            std::cout << "-> ID do usuario proprietario: ";
            if (!(std::cin >> idUsuario)) { limparBuffer(); std::cout << "ID inválido.\n"; break; }

            std::cout << "-> ID do SHA (precisa estar CADASTRADO na opcao 6): ";
            std::cin >> idSHA;

            // valida configuração no DB
            ConfiguracaoSHA cfg = shaConfigDAO.buscarConfiguracao(idSHA);
            if (cfg.idUsuario == -1) {
               std::cout << "❌ Esse SHA nao está cadastrado na tabela ConfiguracaoSHA.\n";
               std::cout << "   Cadastre na opcao 6 primeiro.\n";
               break;
            }

            // se o usuário digitado não bater, usa o do cadastro
            if (cfg.idUsuario != idUsuario) {
               std::cout << "⚠️ Usuario informado (" << idUsuario << ") != Usuario do cadastro (" << cfg.idUsuario << ").\n";
               std::cout << "   Vou usar o usuario do cadastro.\n";
               idUsuario = cfg.idUsuario;
            }

            {
               std::lock_guard<std::mutex> lk(g_muLista);
               if (!listaContemSHA(idSHA)) {
                  g_listaMonitor.push_back({idSHA, idUsuario});
               }
            }

            g_autoAtivo = true;
            std::cout << "✅ Monitoramento automatico ATIVADO para SHA=" << idSHA << " (Usuario=" << idUsuario << ")\n";
         }
         else if (op == 2) {
            std::cout << "-> ID do SHA para remover: ";
            std::cin >> idSHA;

            {
               std::lock_guard<std::mutex> lk(g_muLista);
               g_listaMonitor.erase(
                  std::remove_if(g_listaMonitor.begin(), g_listaMonitor.end(),
                                 [&](const MonitorItem& it){ return it.idSHA == idSHA; }),
                  g_listaMonitor.end()
               );
               if (g_listaMonitor.empty()) g_autoAtivo = false;
            }

            std::cout << "✅ Removido (se existia). Auto ativo = " << (g_autoAtivo.load() ? "SIM" : "NAO") << "\n";
         }
         else if (op == 3) {
            {
               std::lock_guard<std::mutex> lk(g_muLista);
               g_listaMonitor.clear();
            }
            g_autoAtivo = false;
            std::cout << "✅ Monitoramento automatico DESLIGADO (todos removidos).\n";
         }
         else if (op == 4) {
            listarMonitorados();
         }
         else {
            std::cout << "Opção inválida.\n";
         }

         break;
      }

      case 8: {
         int seg;
         std::cout << "--- Ajustar Intervalo do Monitoramento ---\n";
         std::cout << "-> Intervalo em segundos (>=1): ";
         if (!(std::cin >> seg)) { limparBuffer(); std::cout << "Valor inválido.\n"; break; }
         if (seg < 1) seg = 1;
         g_intervaloSeg = seg;
         std::cout << "✅ Intervalo ajustado para " << seg << "s.\n";
         break;
      }

      default:
         std::cout << "Opção inválida.\n";
   }
}

// ========================
// Inicialização
// ========================
void inicializarSistema() {
   std::signal(SIGINT, handleSigint);

   DBConnection conn;

   ConsumoHistoricoDAO cDao(&conn);
   LimiteAlertaDAO lDao(&conn);
   SHAConfigDAO shaConfigDAO(&conn);

   UsuarioDAOImpl uDaoImpl(&conn);
   SubsistemaUsuarios subsUsuarios(&uDaoImpl);

   // construtor correto (1 argumento)
   SubsistemaAlerta subsAlerta(&lDao);

   LeitorImagemSHA leitor(&shaConfigDAO);
   SubsistemaDados subsDados(&leitor, &subsAlerta, &cDao, &shaConfigDAO);

   MonitoramentoFacade fachada(&subsUsuarios, &subsAlerta, &subsDados);

   // Observer do painel
   PainelObserverCLI obs;
   subsAlerta.adicionarObservador(&obs);

   // thread auto (não ativa por padrão)
   std::thread th(loopAuto, std::ref(fachada));

   int comando = -1;
   while (comando != 0 && g_rodando.load()) {
      exibirMenu();
      if (!(std::cin >> comando)) {
         std::cin.clear();
         std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
         comando = -1;
         continue;
      }
      if (comando != 0) {
         processarComando(comando, fachada, shaConfigDAO, uDaoImpl);
      }
   }

   // encerra
   g_rodando = false;
   g_autoAtivo = false;

   if (th.joinable()) th.join();

   subsAlerta.removerObservador(&obs);
}

int main() {
   inicializarSistema();
   return 0;
}
