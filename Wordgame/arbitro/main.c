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
    }
    ReleaseMutex(threadData->hMutex);
    
    return 1;
}



int ExcluirJogador(ThreadDados* threadData, TCHAR* username) {
   DWORD n;
  
   for (int i = 0; i < threadData->nJogadores; i++) {
        RemoveNovaLinha(threadData->jogadores[i].username);
        if (_tcscmp(threadData->jogadores[i].username, username) == 0) {
            if (threadData->hPipes[i].activo) {
                WaitForSingleObject(threadData->hMutex, INFINITE);
                threadData->hPipes[i].activo = FALSE;
                threadData->jogadores[i].ativo = FALSE;
                ReleaseMutex(threadData->hMutex);
                if (!WriteFile(threadData->hPipes[i].hInstancia, _T("EXCLUIDO"), (DWORD)(_tcslen(_T("EXCLUIDO")) * sizeof(TCHAR)), &n, NULL)) {
                    _tprintf(TEXT("[ERRO] - Escrever no pipe! (WriteFile) %d \n"), GetLastError());
                }
               // DisconnectNamedPipe(threadData->hPipes[i].hInstancia);
            } 
            return 1;
        }

    }
   return 0;
}


void ImprimirJogadores(ThreadDados* threadData) {
     for (int i = 0; i < threadData->nJogadores; i++) {
       if(threadData->hPipes[i].activo)
            _tprintf(TEXT("Jogador %d: %s - Pontuação: %.2f\n"), i + 1, threadData->jogadores[i].username, threadData->jogadores[i].pontuacao);
    }
}


DWORD WINAPI threadTrataCliente(LPVOID param) {
    ThreadParams* params = (ThreadParams*)param;
    BOOL ret, continua = TRUE;
    Comandos_Jogador comandos;
    Jogador jogador;
    DWORD n;
    TCHAR aceite[] = _T("ACEITE");
    DWORD jogadores_ativos = 0;

    do {
         ret = ReadFile(params->dados->hPipes[params->jogadorIndex].hInstancia, &comandos, sizeof(comandos), &n, NULL);
            if (!ret) {
                _tprintf(TEXT("[ERRO] - Falha ao ler do pipe. Código de erro: %d\n"), GetLastError());
                continua = FALSE;
                break;
         }
     
        switch (comandos.tipo_comando) {
        case 1:
            //INICIO
            ret = ReadFile(params->dados->hPipes[params->jogadorIndex].hInstancia, &jogador, sizeof(jogador), &n, NULL);

            if (ret != FALSE) {
                _tprintf(TEXT("[ARBITRO] - Recebi %d bytes de dados de login do cliente.\n"), n);
                _tprintf(TEXT("[ARBITRO] - Username: %s\n"), jogador.username);
                jogador.pontuacao = 0;
                params->dados->JogadorIndex = params->jogadorIndex;

                if (AdicionarJogador(params->dados,jogador) == -1) {
                    _tprintf(TEXT("[ARBITRO] - Jogador já existe com este nome: %s\n"), jogador.username);
                    
                    if (!WriteFile(params->dados->hPipes[params->jogadorIndex].hInstancia, _T("NACEITE"), (DWORD)(_tcslen(_T("NACEITE")) * sizeof(TCHAR)), &n, NULL)) {
                        _tprintf(TEXT("[ERRO] - Escrever no pipe! (WriteFile) %d \n"), GetLastError());
                    } 

                    continua = FALSE;
                }
                else {
                    if (!WriteFile(params->dados->hPipes[params->jogadorIndex].hInstancia, _T("ACEITE"), (DWORD)(_tcslen(_T("ACEITE")) * sizeof(TCHAR)), &n, NULL)) {
                        _tprintf(TEXT("[ERRO] - Escrever no pipe! (WriteFile) %d \n"), GetLastError());
                    } 

                    params->dados->jogadores[params->jogadorIndex].ativo = TRUE;
                }
            }
            break;
        case 2:
            //PONTUACAO
            if (!WriteFile(params->dados->hPipes[params->jogadorIndex].hInstancia, _T("PONTUACAO"), (DWORD)(_tcslen(_T("PONTUACAO")) * sizeof(TCHAR)), &n, NULL)) {
                _tprintf(TEXT("[ERRO] - Escrever no pipe! (WriteFile) %d \n"), GetLastError());
            }


            if (!WriteFile(params->dados->hPipes[params->jogadorIndex].hInstancia, &jogador.pontuacao,sizeof(jogador.pontuacao),&n, NULL)) {
                _tprintf(TEXT("[ERRO] - Escrever no pipe! (WriteFile) %d \n"), GetLastError());
            }
            break;
        case 3:
             //JOGADORES
             
              if (!WriteFile(params->dados->hPipes[params->jogadorIndex].hInstancia, _T("JOGS"), (DWORD)(_tcslen(_T("JOGS")) * sizeof(TCHAR)), &n, NULL)) {
                    _tprintf(TEXT("[ERRO] - Escrever no pipe! (WriteFile) %d \n"), GetLastError());
              }


              for (int i = 0; i < params->dados->nJogadores; i++) {
                  jogadores_ativos++;
              }
                
              if (!WriteFile(params->dados->hPipes[params->jogadorIndex].hInstancia,
                  &jogadores_ativos,
                  sizeof(DWORD),
                  &n,
                  NULL)) {
                  _tprintf(TEXT("[ERRO] - Escrever nJogadores no pipe! %d\n"), GetLastError());
              }

              if (!WriteFile(params->dados->hPipes[params->jogadorIndex].hInstancia,
                  &params->dados->jogadores,
                  sizeof(params->dados->jogadores),
                  &n,
                  NULL)) {
                  // erro ao escrever
              }

             break;
        case 4:
            //JOGADOR EXCLUIDO
            _tprintf(TEXT("[ARBITRO] - Saiu Jogador: %s\n"), jogador.username);
             continua = FALSE;
            break;

        }
    } while (continua);
    return 0;
}

DWORD WINAPI threadInterface(LPVOID param) {
    ThreadDados *dados = (ThreadDados*)param;
    TCHAR comando[MAX], ** comandoArray = NULL;
    TCHAR nomeFicheiro[MAX];
    DWORD nArgumentos = 0;
    BOOL continua = TRUE;
    TCHAR username [MAX];

    do {
        _tprintf(_T("\nInsira Comando: "));
        _fgetts(comando, MAX, stdin);
        comando[_tcslen(comando) - 1] = _T('\0');

        if (comandoArray != NULL) {
            free(comandoArray);
            comandoArray = NULL;
        }

        if (_tcslen(comando) > 0) {
            comandoArray = splitString(comando, _T(" "), &nArgumentos);
                if (_tcscmp(comandoArray[0], _T("listar")) == 0) {
                    _tprintf(_T("\nLISTAR JOGADORES\n"));
                    ImprimirJogadores(dados);
                }
                else if (_tcscmp(comandoArray[0], _T("excluir")) == 0 && nArgumentos == 2) {
                    _tcsncpy_s(username, _countof(username), comandoArray[1], _countof(username) - 1);
                    username[_countof(username) - 1] = _T('\0');
                    if (ExcluirJogador(dados, username))
                        _tprintf(_T("\n JOGADOR EXCLUIDO : %s \n"), comandoArray[1]);

                }
                else if (_tcscmp(comandoArray[0], _T("iniciarbot")) == 0 && nArgumentos == 2) {

                    _tprintf(_T("\n Iniciar BOT : %s \n"), comandoArray[1]);

                }
                else if (_tcscmp(comandoArray[0], _T("acelerar")) == 0) {
					WaitForSingleObject(dados->hMutex, INFINITE);
                    if (dados->config.ritmo > 1) {
                        dados->config.ritmo--;
                        _tprintf(_T("\nRitmo -> %d\n"), dados->config.ritmo);
                    }
                    else {
						dados->config.ritmo = 1;
                        _tprintf(_T("\nRitmo -> %d (Máximo ritmo)\n"), dados->config.ritmo);
                    }
					ReleaseMutex(dados->hMutex);
                }
                else if (_tcscmp(comandoArray[0], _T("travar")) == 0) {
					WaitForSingleObject(dados->hMutex, INFINITE);
					dados->config.ritmo++;
					ReleaseMutex(dados->hMutex);
                    _tprintf(_T("\nRitmo -> %d\n"), dados->config.ritmo);
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
    ThreadDados dados;
    MEMDATA memdata;
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
        return -1;
	}

    /*---- VALORES REGISTRY ----*/
    if (!getValueFromKeyMAXLETRAS(&dados.config.max_letras)) {
        _ftprintf(stderr, _T("[ERRO] - Não foi possível obter o valor de MAXLETRAS"));
        dados.config.max_letras = DEFAULT_MAXLETRAS;
        // guardar numa key o valor default
		setValueToKeyMAXLETRAS(DEFAULT_MAXLETRAS);
    }
    else if (dados.config.max_letras > MAXIMO_LETRAS) {
		_ftprintf(stderr, _T("[ERRO] - O valor de MAXLETRAS não pode ser superior a %d"), MAXIMO_LETRAS);
        dados.config.max_letras = MAXIMO_LETRAS;
		setValueToKeyMAXLETRAS(MAXIMO_LETRAS);
    }

    if (!getValueFromKeyRITMO(&dados.config.ritmo)) {
		_ftprintf(stderr, _T("[ERRO] - Não foi possível obter o valor de RITMO"));
        dados.config.ritmo = DEFAULT_RITMO;
		setValueToKeyRITMO(DEFAULT_RITMO);
    }else if(dados.config.ritmo < 1){
		_ftprintf(stderr, _T("[ERRO] - O valor de RITMO não pode ser inferior a 1"));
        dados.config.ritmo = DEFAULT_RITMO;
		setValueToKeyRITMO(DEFAULT_RITMO);
	}

    dados.nJogadores = 0;

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

     dados.hPipes = malloc(DEFAULT_MAX_JOGADORES * sizeof(DadosPipe)); // Aloca memória para os arrays hPipes
     dados.hEvents = malloc(DEFAULT_MAX_JOGADORES * sizeof(HANDLE)); // Aloca memória para os arrays hEvents
     hThreads = malloc(DEFAULT_MAX_JOGADORES * sizeof(HANDLE)); // Aloca memória para os arrays hThreads
     dados.JogadorIndex = 0; //Índice de Cliente no Array hPipes
     dados.terminar = 0; //Flag como sinal de  terminar
   


     /*           Criação de Mutex              */
     dados.hMutex = CreateMutex(NULL, FALSE, NULL);

     if (dados.hMutex == NULL) {
         _tprintf(TEXT("[ERRO] - Criar Mutex! (CreateMutex)\n"));
         CloseHandle(hSemaphoreArbitro);
         CloseHandle(memdata.hMapFile);
         CloseHandle(memdata.hEvent);
         CloseHandle(memdata.hMutex);
         return -1;
     }

     /*           Criação de Evento              */
     for (int i = 0; i < DEFAULT_MAX_JOGADORES; i++) {
         hEventTemp = CreateEvent(NULL, TRUE, FALSE, NULL);

         if (hEventTemp == NULL) {
             _tprintf(TEXT("[ERRO] - Criar Evento! (CreateEvent)\n"));
             CloseHandle(hSemaphoreArbitro);
             CloseHandle(memdata.hMapFile);
             CloseHandle(memdata.hEvent);
             CloseHandle(memdata.hMutex);
             CloseHandle(dados.hMutex);
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
             CloseHandle(dados.hMutex);
             CloseHandle(hEventTemp);
             exit(-1);
         }


         ZeroMemory(&dados.hPipes[i].overlap, sizeof(dados.hPipes[i].overlap));
         dados.hPipes[i].hInstancia = hPipe;
         dados.hPipes[i].overlap.hEvent = hEventTemp;
         dados.hEvents[i] = hEventTemp;
         dados.hPipes[i].activo = FALSE;

         if (ConnectNamedPipe(hPipe, &dados.hPipes[i].overlap)) {
             _tprintf(TEXT("[ERRO] Ligar ao Jogador! (ConnectNamedPipe)\n"));
             CloseHandle(hSemaphoreArbitro);
             CloseHandle(memdata.hMapFile);
             CloseHandle(memdata.hEvent);
             CloseHandle(memdata.hMutex);
             CloseHandle(dados.hMutex);
             exit(-1);
         }
     }

    
     //criar thread interface
     hThreadInterface = CreateThread(NULL, 0, threadInterface, &dados, 0, NULL);

     if (hThreadInterface == NULL) {
         _tprintf(TEXT("[ERRO] - Criar ThreadInterface! (CreateThread)\n"));
         CloseHandle(hSemaphoreArbitro);
         CloseHandle(memdata.hMapFile);
         CloseHandle(memdata.hEvent);
         CloseHandle(memdata.hMutex);
         CloseHandle(dados.hMutex);
         return -1;
     }


     _tprintf(TEXT("[ARBITRO] - Esperar ligacão de um Jogador/Bot...\n"));
     while (!dados.terminar && dados.nJogadores < DEFAULT_MAX_JOGADORES) {
         offset = WaitForMultipleObjects(DEFAULT_MAX_JOGADORES, dados.hEvents, FALSE, INFINITE);
         i = offset - WAIT_OBJECT_0;

         if (i >= 0 && i < DEFAULT_MAX_JOGADORES) {
             _tprintf(TEXT("[ARBITRO] - Jogador/Bot %d entrou...\n"), i);
             if (GetOverlappedResult(dados.hPipes[i].hInstancia,
                 &dados.hPipes[i].overlap, &nBytes, FALSE)) {

                 ResetEvent(dados.hEvents[i]);
                 WaitForSingleObject(dados.hMutex, INFINITE);
                 dados.hPipes[i].activo = TRUE;
                 ReleaseMutex(dados.hMutex);

                 ThreadParams* params = malloc(sizeof(ThreadParams));
                 params->jogadorIndex = i;
                 params->dados = &dados; 

                 // Criar a thread
                 hThreads[dados.nJogadores] = CreateThread(NULL, 0, threadTrataCliente, params, 0, NULL);
                 if (hThreads[dados.nJogadores] == NULL) {
                     _tprintf(TEXT("[ERRO] - Criar Thread! (CreateThread)\n"));
                     free(params);
                     exit(-1);
                 }
                 dados.nJogadores++;
             }
         }
     }

     WaitForSingleObject(hThreadInterface, INFINITE);

     for (int j = 0; j <= dados.nJogadores - 1; j++) {
         WaitForSingleObject(hThreads[j], 1000);
         CloseHandle(hThreads[j]);
     }
     

     _tprintf(TEXT("[BOLSA] - A Desligar pipes e handles\n"));

     for (i = 0; i < dados.nJogadores; i++) {
         if (!DisconnectNamedPipe(dados.hPipes[i].hInstancia)) {
             _tprintf(TEXT("[ERRO] - Desligar o pipe! (DisconnectNamedPipe)\n"));
             exit(-1);
         }
         CloseHandle(dados.hPipes[i].hInstancia);
     }

     free(dados.hPipes);
     free(dados.hEvents);
     free(hThreads);

     CloseHandle(hThreadInterface);
     CloseHandle(hSemaphoreArbitro);    // Liberta semáforo
     CloseHandle(memdata.hMapFile);   // Liberta memória compartilhada 
     CloseHandle(memdata.hEvent);     // Liberta Evento
     CloseHandle(memdata.hMutex);     // Liberta mutex
	return 0;
}