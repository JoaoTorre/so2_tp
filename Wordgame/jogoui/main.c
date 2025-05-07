 #include "utils.h"
#include "struct.h"


int verfica_comandos(Comandos_Jogador *comandos_jogador) {
    TCHAR comando[MAX];
        _tprintf(_T("\nInsira Comando: "));
        _fgetts(comando, MAX, stdin);
        comando[_tcslen(comando) - 1] = _T('\0');

        if (_tcscmp(comando, _T(":pont")) == 0) {
            //Envio de Indentificação Comando
            comandos_jogador->comando[0] = _T("pont");
            comandos_jogador->tipo_comando = 2;
            _tprintf(_T("\nLISTAR PONTUACAO\n"));
        }
        else if (_tcscmp(comando, _T(":jogs")) == 0) {
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

        // Receber a resposta
        ret = ReadFile(hPipe, Resposta->INICIO, 100 * sizeof(TCHAR), &n, NULL);

        if (!ret) {
            _tprintf(TEXT("[Erro] - Ao receber mensagem do ARBITRO: %d\n"), GetLastError());
            return FALSE;
        }

        Resposta->INICIO[n / sizeof(TCHAR)] = '\0';
    }

    // TIPO 2 - PONTUAÇÃO
    if (comandos_jogador->tipo_comando == 2) {
        // Recebendo a resposta
        ret = ReadFile(hPipe, &Resposta->PONTUACAO,sizeof(Resposta->PONTUACAO), &n, NULL);

        if (!ret) {
            _tprintf(TEXT("[Erro] - Ao receber mensagem do ARBITRO: %d\n"), GetLastError());
            return FALSE;
        }
    }
    
    return TRUE;
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

    
    //Envio de Indentificação Jogador
    comandos_jogador.comando[0] = _T("Login");
    comandos_jogador.tipo_comando = 1;

    if (!troca_dados(&jogador, hPipe, &comandos_jogador, &respostas)) {
        _tprintf(TEXT("[Erro] Receber resposta do Arbitro: %s\n"), respostas.INICIO);
        CloseHandle(hPipe);
        return -1;
    }
        

     if (_tcscmp(respostas.INICIO, TEXT("ACEITE")) == 0) {
         _tprintf(TEXT("Inicio de Sessão com sucesso!\n"));

         do {
             if (verfica_comandos(&comandos_jogador)) {
                 if (!troca_dados(NULL, hPipe, &comandos_jogador, &respostas)) {
                     _tprintf(TEXT("[Erro] Receber resposta do Arbitro: %f\n"), respostas.PONTUACAO);
                     CloseHandle(hPipe);
                     return -1;
                 }
                 _tprintf(TEXT("%f\n"), respostas.PONTUACAO);
             }
         } while (continua);

     }
     else if (_tcscmp(respostas.INICIO, TEXT("NACEITE")) == 0) {
         _tprintf(TEXT("Jogador não aceite!\n"));
         _tprintf(TEXT("Pressione qualquer tecla para sair...\n"));
         _getch(); 
         CloseHandle(hPipe);
     }

     CloseHandle(hPipe);
    return 0;
}
