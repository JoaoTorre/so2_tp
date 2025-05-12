 #include "utils.h"
#include "struct.h"


int verfica_comandos(Comandos_Jogador *comandos_jogador, HANDLE * hMutex) {
    TCHAR comando[MAX];
        _tprintf(_T("\nInsira Comando: "));
        _fgetts(comando, MAX, stdin);
        comando[_tcslen(comando) - 1] = _T('\0');

        if (_tcscmp(comando, _T(":pont")) == 0) {
            //Envio de Indentificação Comando
            WaitForSingleObject(hMutex, INFINITE);
            wcscpy_s(comandos_jogador->comando, _countof(comandos_jogador->comando), _T("PONT"));
            comandos_jogador->tipo_comando = 2;
            ReleaseMutex(hMutex);

            _tprintf(_T("\nLISTAR PONTUACAO\n"));
        }
        else if (_tcscmp(comando, _T(":jogs")) == 0) {
            //Envio de Indentificação Comando
            WaitForSingleObject(hMutex, INFINITE);
            wcscpy_s(comandos_jogador->comando,_countof(comandos_jogador->comando),_T("JOGS"));
           comandos_jogador->tipo_comando = 3;
            ReleaseMutex(hMutex);
            _tprintf(_T("\nLISTAR JOGADORES\n"));
        }
        else if (_tcscmp(comando, _T(":sair")) == 0) {
            _tprintf(_T("\nSAIR JOGO\n"));
        }
        else {
            _tprintf(_T("\n[JOGOUI] - Comando inválido (%s)"), comando);
            return FALSE;
        }
  return TRUE;
}

int troca_dados(LPVOID estrutura_enviar, HANDLE hPipe, LPVOID estrutura_comando, Respostas *Resposta) {
    DWORD ret, n;
    Comandos_Jogador* comandos_jogador = (Comandos_Jogador*)estrutura_comando;
    DWORD tamanho_estrutura = 0;

   
    // Envio do tipo de comando
    ret = WriteFile(hPipe, comandos_jogador, sizeof(Comandos_Jogador), &n, NULL);
    if (!ret) {
        _tprintf(TEXT("[Erro] ao enviar identificador do tipo de comando: %d\n"), GetLastError());
        return FALSE;
    }

    // TIPO 1 - INICIO
    if (comandos_jogador->tipo_comando == 1) {
        Jogador* estrutura = (Jogador*)estrutura_enviar;
        tamanho_estrutura = sizeof(Jogador);

        // Envio da estrutura de dados 
        if (!WriteFile(hPipe, estrutura_enviar, tamanho_estrutura, &n, NULL)) {
            _tprintf(TEXT("[Erro] - Ao enviar mensagem do ARBITRO:: %d\n"), GetLastError());
            return FALSE;
        }
    }

    return TRUE;
}

DWORD WINAPI threadArbitro(LPVOID param) {
    ThreadEscutaParam* dados = (ThreadEscutaParam*)param;
    DWORD n;
    BOOL loginEnviado = FALSE;
   
    while (*dados->Continua) {
        
        if (!loginEnviado) {
            WaitForSingleObject(dados->hMutex, INFINITE);
            wcscpy_s(dados->comandos->comando, _countof(dados->comandos->comando), _T("LOGIN"));
            dados->comandos->tipo_comando = 1;
            ReleaseMutex(dados->hMutex);

            if (!troca_dados(dados->jogador, *dados->hPipe, dados->comandos, &dados->respostas)) {
                _tprintf(TEXT("[Erro] Receber resposta do Arbitro: %s\n"), dados->respostas.INICIO);
                return FALSE;
            }
            loginEnviado = TRUE;
        }
       
        BOOL ret  = ReadFile(*dados->hPipe,dados->respostas.INICIO, 100 * sizeof(TCHAR), &n, NULL);
        if (!ret) {
            _tprintf(TEXT("[ERRO] - Falha ao ler do pipe. Código de erro: %d\n"), GetLastError());
            *dados->Continua = FALSE;
            break;
        }

        dados->respostas.INICIO[n / sizeof(TCHAR)] = _T('\0');

        if (_tcscmp(dados->respostas.INICIO, _T("EXCLUIDO")) == 0) {
            WaitForSingleObject(dados->hMutex, INFINITE);
            ZeroMemory(dados->comandos, sizeof(Comandos_Jogador));
            wcscpy_s(dados->comandos->comando, _countof(dados->comandos->comando),_T("EXCLUIDO"));
            dados->comandos->tipo_comando = 4;
            ReleaseMutex(dados->hMutex);
            ret = WriteFile(*dados->hPipe, dados->comandos, sizeof(Comandos_Jogador), &n, NULL);
            if (!ret) {
                _tprintf(TEXT("[Erro] ao enviar identificador do tipo de comando: %d\n"), GetLastError());
                return FALSE;
            }
     
            break;
        }

        if (_tcscmp(dados->respostas.INICIO, TEXT("ACEITE")) == 0) {
            _tprintf(TEXT("Inicio de Sessão com sucesso!\n"));
        }

        if (_tcscmp(dados->respostas.INICIO, TEXT("NACEITE")) == 0) {
            _tprintf(TEXT("Jogador não aceite!\n"));
            *dados->Continua = FALSE;
        }

        if (_tcscmp(dados->respostas.INICIO, _T("PONTUACAO")) == 0) {
            // Recebendo a resposta
            ret = ReadFile(*dados->hPipe, &dados->respostas.jogador.PONTUACAO, sizeof(dados->respostas.jogador.PONTUACAO), &n, NULL);

            if (!ret) {
                _tprintf(TEXT("[Erro] - Ao receber mensagem do ARBITRO: %d\n"), GetLastError());
                return FALSE;
            }

            _tprintf(TEXT("%.2f\n"), dados->respostas.jogador.PONTUACAO);
        }


        if (_tcscmp(dados->respostas.INICIO, _T("JOGS")) == 0) {

            if (!ReadFile(*dados->hPipe, &dados->respostas.nJogadores, sizeof(DWORD), &n, NULL)) {
                _tprintf(TEXT("[Erro] - Ao receber número de jogadores: %d\n"), GetLastError());
                return FALSE;
            }


            ret = ReadFile(*dados->hPipe,
                &dados->respostas.ListaJogadores,
                sizeof(dados->respostas.ListaJogadores),
                &n, NULL);

            if (!ret || n != sizeof(dados->respostas.ListaJogadores)) {
                _tprintf(TEXT("[Erro] - Ao receber jogadores: %d\n"), GetLastError());
                return FALSE;
            }
  
        }
    }

    _tprintf(_T("[JOGADOR] - O jogo foi encerrado.\n"));
     _tprintf(TEXT("Pressione qualquer tecla para sair...\n"));
    *dados->Continua = FALSE;
    return 0;
}


int _tmain(int argc, LPTSTR argv[]) {
    HANDLE hPipe;
    Jogador jogador;
    Comandos_Jogador comandos_jogador;
    BOOL continua = TRUE;
    DWORD  n;
    unsigned int i = 0;
    BOOL ret = FALSE;
    TCHAR Resposta_Login[MAX];
    TCHAR username[MAX];
    Respostas respostas;
    HANDLE ThreadArbitro;
    ThreadEscutaParam threadescuta;

#ifdef UNICODE
    _setmode(_fileno(stderr), _O_WTEXT);
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
#endif
    
    _tprintf_s(_T("Insira nome de utilizador: "));
    _fgetts(jogador.username, MAX, stdin);
   

    // abrir pipe; esperar se necessário
    while (1) {
        hPipe = CreateFile(
            name_pipe, // nome do pipe
            GENERIC_READ | GENERIC_WRITE, // acesso de leitura e escrita*
            0,
            NULL,   // ATRIBUTOS DE SEGURANÇA
            OPEN_EXISTING, // ABRIR UM PIPE QUE EXISTA
            FILE_FLAG_OVERLAPPED,  // OPERAÇÕES ASSÍNCRONAS
            NULL // SEM FICHEIRO DE TEMPLATE
        );

        if (hPipe != INVALID_HANDLE_VALUE) {
            break;
        }

        if (GetLastError() != ERROR_PIPE_BUSY) {
            _tprintf_s(_T("[ERRO] - Não foi possível abrir o pipe. GLE=%d\n"), GetLastError());
            return -1;
        }

        if (!WaitNamedPipe(name_pipe, 20000)) {
            _tprintf_s(_T("[ERRO] - Não foi possível abrir pipe: 20 segundos timed out\n"));
            return -1;
        }
    }

         HANDLE hMutex = CreateMutex(NULL, FALSE, NOME_MUTEX);
            if (hMutex == NULL) {
                printf("Erro ao criar mutex: %lu\n", GetLastError());
                return 1;
        }

         threadescuta.jogador = &jogador;
         threadescuta.Continua = &continua;
         threadescuta.hPipe = &hPipe;
         threadescuta.respostas = respostas;
         threadescuta.comandos = &comandos_jogador;
         threadescuta.hMutex = &hMutex;

         ThreadArbitro = CreateThread(NULL, 0, threadArbitro, &threadescuta, 0, NULL);

         if (ThreadArbitro == NULL) {
             _tprintf(TEXT("[ERRO] - Criar ThreadInterface! (CreateThread)\n"));
             CloseHandle(hPipe);
             return -1;
         }
        
         do {
             if (verfica_comandos(&comandos_jogador,hMutex)) {
                 if (!troca_dados(NULL, hPipe, &comandos_jogador, &respostas)) {
                     _tprintf(TEXT("[Erro] Receber resposta do Arbitro: %f\n"), respostas.jogador.PONTUACAO);
                     CloseHandle(hPipe);
                     return -1;
                 }
             }

         } while (continua);

      
         WaitForSingleObject(ThreadArbitro, INFINITE);
         CloseHandle(ThreadArbitro);
         CloseHandle(hMutex);
         CloseHandle(hPipe);

      
        
    return 0;
}
