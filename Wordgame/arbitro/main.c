#include "utils.h"
#include "struct.h"


int AdicionarJogador(ThreadDados* threadData, Jogador novoJogador) {

    for (int i = 0; i < threadData->nJogadores; i++) {
        if (_tcscmp(threadData->jogadores[i].username, novoJogador.username) == 0) {
            return -1;
        }
    }

    WaitForSingleObject(threadData->hMutex, INFINITE);
    if (threadData->nJogadores < DEFAULT_MAX_JOGADORES) {
        threadData->jogadores[threadData->JogadorIndex] = novoJogador;  
        threadData->nJogadores++;  
    }
    ReleaseMutex(threadData->hMutex);
}


void ImprimirJogadores(ThreadDados* threadData) {

    for (int i = 0; i < threadData->nJogadores; i++) {
        _tprintf(TEXT("Jogador %d: %s - Pontuação: %d\n"),i + 1,threadData->jogadores[i].username, threadData->jogadores[i].pontuacao);
    }
}


DWORD WINAPI threadTrataCliente(LPVOID param) {
    ThreadDados* dados = (ThreadDados*)param;
    BOOL ret, continua = TRUE;
    Comandos_Jogador comandos;
    Jogador jogador;
    DWORD n;

    do {
        ret = ReadFile(dados->hPipes[dados->JogadorIndex].hInstancia, &comandos.tipo_comando, sizeof(comandos.tipo_comando), &n, NULL);

        switch (comandos.tipo_comando) {
            case 1:
                //INICIO
                ret = ReadFile(dados->hPipes[dados->JogadorIndex].hInstancia, &jogador, sizeof(jogador), &n, NULL);

                if (ret != FALSE) {
                    _tprintf(TEXT("[ARBITRO] - Recebi %d bytes de dados de login do cliente.\n"), n);
                    _tprintf(TEXT("[ARBITRO] - Username: %s\n"), jogador.username);
                    jogador.pontuacao = 0;
                    if (AdicionarJogador(dados, jogador) == -1) {
                        _tprintf(TEXT("[ARBITRO] - Jogador já existe com este nome: %s\n"), jogador.username);
                   }
                }
           break;

        }
    }while (continua);
    return 0;
}


DWORD WINAPI threadInterface(LPVOID param) {
    ThreadDados* dados = (ThreadDados*)param;
    TCHAR comando[MAX], ** comandoArray = NULL;
    TCHAR nomeFicheiro[MAX];
    DWORD nArgumentos = 0;
    BOOL continua = TRUE;

    do {
        _tprintf(_T("\nInsira Comando: "));
        _fgetts(comando, MAX, stdin);
        comando[_tcslen(comando) - 1] = _T('\0');

        free(comandoArray);

        if (_tcslen(comando) > 0) {
            comandoArray = splitString(comando, _T(" "), &nArgumentos);

                if (_tcscmp(comandoArray[0], _T("listar")) == 0) {
                    _tprintf(_T("\nLISTAR JOGADORES\n"));
                    ImprimirJogadores(dados);
                }
                else if (_tcscmp(comandoArray[0], _T("excluir")) == 0 && nArgumentos == 2) {
                    _tprintf(_T("\n Excluir JOGADOR : %s \n"), comandoArray[1]);
                }
                else if (_tcscmp(comandoArray[0], _T("iniciarbot")) == 0 && nArgumentos == 2) {
                    _tprintf(_T("\n Iniciar BOT : %s \n"), comandoArray[1]);

                }
                else if (_tcscmp(comandoArray[0], _T("acelar")) == 0) {
                    _tprintf(_T("\nAcelar\n"));
                }
                else if (_tcscmp(comandoArray[0], _T("travar")) == 0) {
                    _tprintf(_T("\nTravar\n"));
                }
                else if (_tcscmp(comandoArray[0], _T("encerrar")) == 0) {
                    _tprintf(_T("\nEncerrar\n"));
                    continua = FALSE;
                }
                else {
                    _tprintf(_T("\n[ARBITRO] - Comando inválido (%s)"), comandoArray[0]);
                }
           }
        } while (continua);

        free(comandoArray);
        return 0;
}


int _tmain(int argc, TCHAR* argv[]) {

    HANDLE hSemaphoreArbitro, hSemaphoreMaxClientes, hEventTemp, hPipe, hThreadInterface;
    HANDLE* hThreads;
    ThreadDados *dados;
    MEMDATA memdata;
    int numJogadores = 0;
    DWORD offset, nBytes;
    int i;

#ifdef UNICODE 
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif
	
	hSemaphoreArbitro = CreateSemaphore(NULL, 1, 1, SEMAPHORE_UNIQUE_ARBITRO_NAME);

	if (hSemaphoreArbitro == NULL) {
		_ftprintf(stderr, _T("[ERRO] - Falha ao criar o semáforo: %d\n"), GetLastError());
		return -1;
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		_ftprintf(stderr, _T("[ERRO] - Já existe um Árbitro a decorrer"));
	}


    /*----MEMÓRIA COMPARTILHADA----*/

    // Iniciar arquivo de memória compartilhada
    memdata.hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,   // Usar o arquivo de paginação
        NULL,                   // Segurança padrão
        PAGE_READWRITE,         // Permissão de leitura/escrita
        0,                      // Tamanho máximo alto do objeto
        sizeof(SHAREDMEM_LETRAS), // Tamanho máximo baixo do objeto
        MEMORIA_PARTILHADA_NOME); // Nome do objeto de mapeamento


    if (memdata.hMapFile == NULL) {
        _tprintf_s(_T("[ERRO] - Não foi possível criar arquivo para memória compartilhada (%d).\n"), GetLastError());
        CloseHandle(hSemaphoreArbitro);
        return -1;
    }


    // Criar evento para notificar arbitro
     memdata.hEvent = CreateEvent(NULL, TRUE, FALSE, MEMORIA_PARTILHADA_EVENTO);

     if (memdata.hEvent == NULL) {
         _tprintf_s(_T("[ERRO] - Não foi possível criar evento (%d).\n"), GetLastError());
         CloseHandle(hSemaphoreArbitro);
         CloseHandle(memdata.hMapFile);
         return -1;
     }


     // Criar mutex para escrever na memória
     memdata.hMutex = CreateMutex(NULL, FALSE, MEMORIA_PARTILHADA_MUTEX);

     if (memdata.hMutex == NULL) {
         _tprintf_s(_T("[ERRO] - Não foi possível criar mutex para memória compartilhada (%d).\n"), GetLastError());
         CloseHandle(hSemaphoreArbitro);
         CloseHandle(memdata.hMapFile);
         CloseHandle(memdata.hEvent);
         return -1;
     }

     /*        PIPES           */
     dados = (ThreadDados*)malloc(sizeof(ThreadDados));
     dados->hPipes = malloc(DEFAULT_MAX_JOGADORES * sizeof(DadosPipe)); // Aloca memória para os arrays hPipes
     dados->hEvents = malloc(DEFAULT_MAX_JOGADORES * sizeof(HANDLE)); // Aloca memória para os arrays hEvents
     hThreads = malloc(DEFAULT_MAX_JOGADORES * sizeof(HANDLE)); // Aloca memória para os arrays hThreads
     dados->JogadorIndex = 0; //Índice de Cliente no Array hPipes
     dados->terminar = 0; //Flag como sinal de  terminar
     dados->nJogadores = 0;


     /*           Criação de Mutex              */

     dados->hMutex = CreateMutex(NULL, FALSE, NULL);

     if (dados->hMutex == NULL) {
         _tprintf(TEXT("[ERRO] - Criar Mutex! (CreateMutex)\n"));
         CloseHandle(hSemaphoreArbitro);
         CloseHandle(memdata.hMapFile);
         CloseHandle(memdata.hEvent);
         CloseHandle(memdata.hMutex);
         return -1;
     }

     /*           Criação de Evento              */

     //dados.nJogadores = DEFAULT_MAX_JOGADORES;

     for (int i = 0; i < DEFAULT_MAX_JOGADORES; i++) {
         hEventTemp = CreateEvent(NULL, TRUE, FALSE, NULL);

         if (hEventTemp == NULL) {
             _tprintf(TEXT("[ERRO] - Criar Evento! (CreateEvent)\n"));
             CloseHandle(hSemaphoreArbitro);
             CloseHandle(memdata.hMapFile);
             CloseHandle(memdata.hEvent);
             CloseHandle(memdata.hMutex);
             CloseHandle(dados->hMutex);
             exit(-1);
         }

         /*         Criação de PIPES              */
         hPipe = CreateNamedPipe(NAMEDPIPE_JOGADORES,
             PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
             PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
             DEFAULT_MAX_JOGADORES, 256 * sizeof(TCHAR), 256 * sizeof(TCHAR), 1000, NULL);


         if (hPipe == INVALID_HANDLE_VALUE) {
             _tprintf(TEXT("[ERRO] Criar Named Pipe! (CreateNamedPipe)\n"));
             CloseHandle(hSemaphoreArbitro);
             CloseHandle(memdata.hMapFile);
             CloseHandle(memdata.hEvent);
             CloseHandle(memdata.hMutex);
             CloseHandle(dados->hMutex);
             CloseHandle(hEventTemp);
             exit(-1);
         }


         ZeroMemory(&dados->hPipes[i].overlap, sizeof(dados->hPipes[i].overlap));
         dados->hPipes[i].hInstancia = hPipe;
         dados->hPipes[i].overlap.hEvent = hEventTemp;
         dados->hEvents[i] = hEventTemp;
         dados->hPipes[i].activo = FALSE;

         if (ConnectNamedPipe(hPipe, &dados->hPipes[i].overlap)) {
             _tprintf(TEXT("[ERRO] Ligar ao Jogador! (ConnectNamedPipe)\n"));
             CloseHandle(hSemaphoreArbitro);
             CloseHandle(memdata.hMapFile);
             CloseHandle(memdata.hEvent);
             CloseHandle(memdata.hMutex);
             CloseHandle(dados->hMutex);
             exit(-1);
         }
     }

    

     //criar thread interface
     hThreadInterface = CreateThread(NULL, 0, threadInterface, dados, 0, NULL);

     if (hThreadInterface == NULL) {
         _tprintf(TEXT("[ERRO] - Criar ThreadInterface! (CreateThread)\n"));
         CloseHandle(hSemaphoreArbitro);
         CloseHandle(memdata.hMapFile);
         CloseHandle(memdata.hEvent);
         CloseHandle(memdata.hMutex);
         CloseHandle(dados->hMutex);
         return -1;
     }
     _tprintf(TEXT("[ARBITRO] - Esperar ligacão de um Jogador/Bot...\n"));
     while (!dados->terminar && numJogadores < DEFAULT_MAX_JOGADORES) {
         offset = WaitForMultipleObjects(DEFAULT_MAX_JOGADORES, dados->hEvents, FALSE, INFINITE);
         i = offset - WAIT_OBJECT_0;

         if (i >= 0 && i < DEFAULT_MAX_JOGADORES) {
             _tprintf(TEXT("[ARBITRO] - Jogador/Bot %d entrou...\n"), i);

             if (GetOverlappedResult(dados->hPipes[i].hInstancia,
                 &dados->hPipes[i].overlap, &nBytes, FALSE)) {

                 ResetEvent(dados->hEvents[i]);
                 WaitForSingleObject(dados->hMutex, INFINITE);
                 dados->hPipes[i].activo = TRUE;
                 ReleaseMutex(dados->hMutex);

                
                 WaitForSingleObject(dados->hMutex, INFINITE);
                 dados->JogadorIndex = numJogadores;
                 ReleaseMutex(dados->hMutex);
                 
                 numJogadores++;

                 // Criar a thread
                 hThreads[numJogadores] = CreateThread(NULL, 0, threadTrataCliente, dados,0, NULL);
                 if (hThreads[numJogadores] == NULL) {
                     _tprintf(TEXT("[ERRO] - Criar Thread! (CreateThread)\n"));
                     exit(-1);
                 }     
             }
         }
     }

     WaitForSingleObject(hThreadInterface, INFINITE);

     for (int j = 0; j <= numJogadores - 1; j++) {
         WaitForSingleObject(hThreads[j], 1000);
         CloseHandle(hThreads[j]);
     }

     

     _tprintf(TEXT("[BOLSA] - A Desligar pipes e handles\n"));

     for (i = 0; i < dados->nJogadores; i++) {
         if (!DisconnectNamedPipe(dados->hPipes[i].hInstancia)) {
             _tprintf(TEXT("[ERRO] - Desligar o pipe! (DisconnectNamedPipe)\n"));
             exit(-1);
         }
         CloseHandle(dados->hPipes[i].hInstancia);
     }

     free(dados->hPipes);
     free(dados->hEvents);
     free(hThreads);

     CloseHandle(hThreadInterface);
     CloseHandle(hSemaphoreArbitro);    // Liberta semáforo
     CloseHandle(memdata.hMapFile);   // Liberta memória compartilhada 
     CloseHandle(memdata.hEvent);     // Liberta Evento
     CloseHandle(memdata.hMutex);     // Liberta mutex
	return 0;
}