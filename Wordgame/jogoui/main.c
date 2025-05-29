#include "utils.h"
#include "struct.h"


int verifica_comandos(DadosPartilhados* dadosPartilhados) {
    TCHAR comando[MAX];
    DWORD n;
    HANDLE* hPipe = dadosPartilhados->hPipe;
    HANDLE* hMutex = dadosPartilhados->hMutex;
    Comandos_Jogador* comandos_jogador = dadosPartilhados->comandos;

   // _tprintf(_T("\nInsira Comando: "));
    _fgetts(comando, MAX, stdin);
    comando[_tcslen(comando) - 1] = _T('\0');

    if (_tcscmp(comando, _T(":pont")) == 0) {
        float pont;

        WaitForSingleObject(*hMutex, INFINITE);
        wcscpy_s(comandos_jogador->comando, _countof(comandos_jogador->comando), _T("PONT"));
        comandos_jogador->tipo_comando = 2;
        ReleaseMutex(*hMutex);

        ResetEvent(dadosPartilhados->hEventoAvancar);
        SetEvent(dadosPartilhados->hEventoParar);

        WriteFile(*hPipe, comandos_jogador, sizeof(Comandos_Jogador), &n, NULL);
       
        BOOL ret = ReadFile(*hPipe, &pont, sizeof(float), &n, NULL);
        if (ret && n == sizeof(float)) {
            WaitForSingleObject(dadosPartilhados->hMutex, INFINITE);
            dadosPartilhados->pontuacao = pont;
            ReleaseMutex(dadosPartilhados->hMutex);
            _tprintf(TEXT("\nPontuação atual: %.2f\n"), dadosPartilhados->pontuacao);
        }

        ResetEvent(dadosPartilhados->hEventoParar);
        SetEvent(dadosPartilhados->hEventoAvancar);
        return TRUE;

    }else if (_tcscmp(comando, _T(":jogs")) == 0) {
        int nJogadores;

        WaitForSingleObject(hMutex, INFINITE);
        wcscpy_s(comandos_jogador->comando, _countof(comandos_jogador->comando), _T("JOGS"));
        comandos_jogador->tipo_comando = 3;
        ReleaseMutex(hMutex);
      
        ResetEvent(dadosPartilhados->hEventoAvancar);
        SetEvent(dadosPartilhados->hEventoParar);

        WriteFile(*hPipe, comandos_jogador, sizeof(Comandos_Jogador), &n, NULL);

        ReadFile(*hPipe, &nJogadores, sizeof(int), &n, NULL);

        EnviaDados* jogadoresRecebidos = (EnviaDados*)malloc(sizeof(EnviaDados));
        jogadoresRecebidos->nJogadoresativos = nJogadores;
        jogadoresRecebidos->jogadores = (Jogador*)malloc(nJogadores * sizeof(Jogador));
       
        BOOL ret = ReadFile(*hPipe, jogadoresRecebidos->jogadores, nJogadores * sizeof(Jogador), &n, NULL);

        if (ret && n == nJogadores * sizeof(Jogador)) {
            for (int i = 0; i < nJogadores; i++) {
                _tprintf(TEXT("\n--- Jogador %d ---\n"), i + 1);
                _tprintf(TEXT("Nome      : %s\n"), jogadoresRecebidos->jogadores[i].username);
                _tprintf(TEXT("Pontuação : %.2f\n"), jogadoresRecebidos->jogadores[i].pontuacao);
            }
        }
        free(jogadoresRecebidos->jogadores);
        free(jogadoresRecebidos);
        ResetEvent(dadosPartilhados->hEventoParar);
        SetEvent(dadosPartilhados->hEventoAvancar);
        return TRUE;

    }else if (_tcscmp(comando, _T(":sair")) == 0) {
        WaitForSingleObject(hMutex, INFINITE);
        wcscpy_s(comandos_jogador->comando, _countof(comandos_jogador->comando), _T("SAIR"));
        comandos_jogador->tipo_comando = 4;
        ReleaseMutex(hMutex);
        WriteFile(*hPipe, comandos_jogador, sizeof(Comandos_Jogador), &n, NULL);
        *dadosPartilhados->Continua = FALSE;
        return FALSE;
    }else if (comando[0] == _T(':')) {
        _tprintf(_T("Comando inválido: %s\n"), comando);
        return TRUE; 
    }
    else if (_tcslen(comando) > 0) { 
        if (dadosPartilhados->jogoIniciado == TRUE) {
            float pont;
            DWORD tamanho_palavra;
            int resultado;

            ResetEvent(dadosPartilhados->hEventoAvancar);
            SetEvent(dadosPartilhados->hEventoParar);
            WaitForSingleObject(hMutex, INFINITE);
            wcscpy_s(comandos_jogador->comando, _countof(comandos_jogador->comando), comando);
            comandos_jogador->tipo_comando = 5;
            ReleaseMutex(hMutex);
            WriteFile(*hPipe, comandos_jogador, sizeof(Comandos_Jogador), &n, NULL);
            _tprintf(TEXT("Palavra Enviada : %s\n"), comando);
         
            BOOL ret = ReadFile(*hPipe, &resultado, sizeof(int), &n, NULL);
            if (ret && n == sizeof(int)) {    
                if (resultado == 1) {
                    BOOL ret2 = ReadFile(*hPipe, &pont, sizeof(float), &n, NULL);
                    if (ret2 && n == sizeof(float)) {
                        WaitForSingleObject(dadosPartilhados->hMutex, INFINITE);
                        dadosPartilhados->pontuacao = pont;
                        ReleaseMutex(dadosPartilhados->hMutex);
                        _tprintf(TEXT("\nPontuação atual: %.2f\n"), dadosPartilhados->pontuacao);
                    }

                    BOOL ret3 = ReadFile(*hPipe, &tamanho_palavra, sizeof(DWORD), &n, NULL);
                    if (ret3 && n == sizeof(DWORD)) {
                        dadosPartilhados->jogador->palavra = malloc(tamanho_palavra * sizeof(TCHAR));
                        ReadFile(*hPipe, dadosPartilhados->jogador->palavra, tamanho_palavra * sizeof(TCHAR), &n, NULL);
                        _tprintf(TEXT("\nParabéns acertaste a palavra %s!\n"), dadosPartilhados->jogador->palavra);
                    }

                }else if (resultado == 2) {
                    BOOL ret3 = ReadFile(*hPipe, &tamanho_palavra, sizeof(DWORD), &n, NULL);
                    if (ret3 && n == sizeof(DWORD)) {
                        dadosPartilhados->jogador->palavra = malloc(tamanho_palavra * sizeof(TCHAR));
                        ReadFile(*hPipe, dadosPartilhados->jogador->palavra, tamanho_palavra * sizeof(TCHAR), &n, NULL);
                        _tprintf(TEXT("\nPalavra não encontrada: %s!\n"), dadosPartilhados->jogador->palavra);
                    }
                }
            }

            ResetEvent(dadosPartilhados->hEventoParar);
            SetEvent(dadosPartilhados->hEventoAvancar);
            return TRUE;
        }
        else {
            _tprintf(_T("Jogo ainda não iniciado!\n"));
            return TRUE;
        }     
    }
}

DWORD WINAPI threadArbitro(LPVOID param) {
    ThreadEscutaParam* dados = (ThreadEscutaParam*)param;
    DWORD n;
    BOOL loginEnviado = FALSE;
    DadosPartilhados* dadosPartilhados;  
    dadosPartilhados= dados->dadosPartilhados;
    Comandos_Jogador comandos;
    TCHAR resposta[255];

    if (!loginEnviado) {
        WaitForSingleObject(*dados->dadosPartilhados->hMutex, INFINITE);
        dados->dadosPartilhados->comandos->tipo_comando = 1;
        ReleaseMutex(*dados->dadosPartilhados->hMutex);
        WriteFile(*dados->dadosPartilhados->hPipe,dados->dadosPartilhados->comandos, sizeof(Comandos_Jogador), &n, NULL);
        WriteFile(*dados->dadosPartilhados->hPipe,dados->dadosPartilhados->jogador, sizeof(Jogador), &n, NULL);
        loginEnviado = TRUE;
        SetEvent(dadosPartilhados->hEventoAvancar);
    }

    while (*dados->Continua) {
        DWORD ret = WaitForMultipleObjects(2, (const HANDLE[]) { dados->dadosPartilhados->hEventoAvancar, dados->dadosPartilhados->hEventoParar}, FALSE, INFINITE);

        if (ret == WAIT_OBJECT_0) {
            BOOL ret = ReadFile(*dados->dadosPartilhados->hPipe, dados->header, sizeof(MensagemHeader), &n, NULL);
                if (!ret) {
                    DWORD error = GetLastError();
                    switch (error) {
                    case ERROR_BROKEN_PIPE:
                        _tprintf(TEXT("[JOGOUI] - O jogo foi encerrado"));
                        *dados->Continua = FALSE;
                        break;
                    case ERROR_PIPE_NOT_CONNECTED:
                        _tprintf(TEXT("[JOGOUI] - O jogo foi encerrado\n"));
                        *dados->Continua = FALSE;
                        break;
                    default:
                        continue;
                        break;
                    }    
                }
      
            if (n == sizeof(MensagemHeader)) {
                switch (dados->header->tipo) {
                case 98: {
                        ReadFile(*dados->dadosPartilhados->hPipe, resposta, dados->header->tamanho, &n, NULL);
                        resposta[n / sizeof(TCHAR)] = _T('\0');
                        _tprintf(TEXT("\nSessão iniciada com sucesso: %s\n"), resposta);
                        break;
                }
                case 99: {
                        ReadFile(*dados->dadosPartilhados->hPipe, resposta, dados->header->tamanho, &n, NULL);
                        resposta[n / sizeof(TCHAR)] = _T('\0');
                        _tprintf(TEXT("\nJogador não aceite: %s\n"), resposta);
                        *dados->Continua = FALSE;
                        break;
                }
                case 40: {
                        ReadFile(*dados->dadosPartilhados->hPipe, resposta, dados->header->tamanho, &n, NULL);
                        resposta[n / sizeof(TCHAR)] = _T('\0');
                        *dados->Continua = FALSE;
                        break;
                    }

                case 50: {
                    ReadFile(*dados->dadosPartilhados->hPipe,dados->dadosPartilhados->jogador, sizeof(Jogador), &n, NULL);
                    _tprintf(TEXT("\nO jogador %s saiu do jogo\n"), dados->dadosPartilhados->jogador->username);
                    break;
                }

                case 60: {
                    ReadFile(*dados->dadosPartilhados->hPipe,dados->dadosPartilhados->jogador,sizeof(Jogador), &n, NULL);
                    _tprintf(TEXT("\nO jogador %s entrou no jogo\n"), dados->dadosPartilhados->jogador->username);
                    break;
                }

                case 10: {
                    ReadFile(*dados->dadosPartilhados->hPipe, resposta, dados->header->tamanho, &n, NULL);
                    resposta[n / sizeof(TCHAR)] = _T('\0');
                    WaitForSingleObject(*dados->dadosPartilhados->hMutex, INFINITE);
                    dados->dadosPartilhados->jogoIniciado = TRUE;
                    ReleaseMutex(*dados->dadosPartilhados->hMutex);
                    _tprintf(TEXT("\nJogo iniciado\n"));
                    break;
                }

                case 20: {
                    ReadFile(*dados->dadosPartilhados->hPipe, resposta, dados->header->tamanho, &n, NULL);
                    resposta[n / sizeof(TCHAR)] = _T('\0');
                    WaitForSingleObject(*dados->dadosPartilhados->hMutex, INFINITE);
                    dados->dadosPartilhados->jogoIniciado = FALSE;
                    ReleaseMutex(*dados->dadosPartilhados->hMutex);
                    _tprintf(TEXT("\nJogo Terminou,nº jogadores inferior a um \n"));
                    break;
                }

                case 70: {
                    ReadFile(*dados->dadosPartilhados->hPipe, dados->dadosPartilhados->jogador, sizeof(Jogador), &n, NULL);
                    dados->dadosPartilhados->jogador->palavra = malloc(dados->header->tamanho * sizeof(TCHAR));
                    ReadFile(*dados->dadosPartilhados->hPipe, dados->dadosPartilhados->jogador->palavra, dados->header->tamanho * sizeof(TCHAR), &n, NULL);
                    _tprintf(TEXT("\nO jogador %s acertou a palavra %s\n"), dados->dadosPartilhados->jogador->username, dados->dadosPartilhados->jogador->palavra);
                    break;
                }

                case 63: {
                    ReadFile(*dados->dadosPartilhados->hPipe, dados->dadosPartilhados->jogador, sizeof(Jogador), &n, NULL);
                    _tprintf(TEXT("\nO jogador %s está em primeiro lugar, com pontuação %.2f\n"), dados->dadosPartilhados->jogador->username, dados->dadosPartilhados->jogador->pontuacao);
                    break;
                }

                case 64: {
                    ReadFile(*dados->dadosPartilhados->hPipe, dados->dadosPartilhados->jogador, sizeof(Jogador), &n, NULL);
                    _tprintf(TEXT("\nParabéns estás em primeiro lugar!\n"));
                    break;
                }
             }
              
            }
        }
        
    }

    _tprintf(TEXT("[JOGADORUI] - O jogo foi encerrado.\n"));
    _tprintf(TEXT("Pressione qualquer tecla para sair...\n"));
    *dados->Continua = FALSE;
    return 0;
}

DWORD WINAPI EsperaMemData(LPVOID param){
	SHARED_THREAD* sharedThread = (SHARED_THREAD*)param;
	HANDLE handles[2] = { sharedThread->hEvent, sharedThread->hMutex };
	DWORD dwWaitResult;

    do {
    
		dwWaitResult = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

        if (dwWaitResult == WAIT_FAILED) {
            _tprintf(_T("[ERRO] - WaitForMultipleObjects falhou com erro: %d\n"), GetLastError());
            sharedThread->continuar = FALSE;
        }

        if (dwWaitResult == WAIT_OBJECT_0) {
			sharedThread->pSharedData = (SHAREDMEM_LETRAS*)MapViewOfFile(
				sharedThread->hMapFile,
				FILE_MAP_READ,
				0,
				0,
				sizeof(SHAREDMEM_LETRAS)
			);

            if (sharedThread->pSharedData == NULL) {
                _tprintf(_T("Não foi possível mapear a visão do arquivo (%d).\n"), GetLastError());
                 sharedThread->continuar = FALSE;
            }

            //Copiar a estrutura para uma variavel local
			SHAREDMEM_LETRAS sharedDataCopy;
			CopyMemory(&sharedDataCopy, sharedThread->pSharedData, sizeof(SHAREDMEM_LETRAS));
			
			UnmapViewOfFile(sharedThread->pSharedData);
			ResetEvent(sharedThread->hEvent);
			ReleaseMutex(sharedThread->hMutex);

			for (int i = 0; i < (int)_tcslen(sharedDataCopy.letras_visiveis); i++) {
				_tprintf(_T("%c "), sharedDataCopy.letras_visiveis[i]);
			}
            _tprintf(_T("\n"));
            
        }
        else if (dwWaitResult == WAIT_OBJECT_0 + 1) {

            ReleaseMutex(sharedThread->hMutex);
        }
       

    } while (sharedThread->continuar);


}

int _tmain(int argc, LPTSTR argv[]) {
    HANDLE hPipe;
    Jogador jogador;
    Comandos_Jogador comandos_jogador;
    BOOL continua = TRUE;
    DWORD n;
    BOOL ret = FALSE;
    TCHAR Resposta_Login[MAX];
    TCHAR username[MAX];
    HANDLE ThreadArbitro, hThreadMemData;
    ThreadEscutaParam threadescuta;
    MensagemHeader header;
	SHARED_THREAD sharedThread;

#ifdef UNICODE
    _setmode(_fileno(stderr), _O_WTEXT);
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
#endif

    _tprintf_s(_T("Insira nome de utilizador: "));
    _fgetts(jogador.username, MAX, stdin);
    jogador.bot = FALSE;

    while (1) {
        hPipe = CreateFile(
            name_pipe,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            NULL
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

    HANDLE hMutex = CreateMutex(NULL, FALSE, NULL);

    if (hMutex == NULL) {
        printf("Erro ao criar mutex: %lu\n", GetLastError());
        return 1;
    }


    /* dados para a memoria partilhada */
    sharedThread.hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MEMORIA_PARTILHADA_MUTEX);

	if (sharedThread.hMutex == NULL) {
		_tprintf_s(_T("[ERRO] - Não foi possível abrir o mutex de memória partilhada (%d).\n"), GetLastError());
		CloseHandle(hPipe);
		return -1;
	}

    sharedThread.hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, MEMORIA_PARTILHADA_EVENTO);

	if (sharedThread.hEvent == NULL) {
		_tprintf_s(_T("[ERRO] - Não foi possível abrir o evento de memória partilhada (%d).\n"), GetLastError());
		CloseHandle(sharedThread.hMutex);
		CloseHandle(hPipe);
		return -1;
	}

    sharedThread.hMapFile = OpenFileMapping(
        FILE_MAP_READ,
        FALSE,
        MEMORIA_PARTILHADA_NOME
    );

    if (sharedThread.hMapFile == NULL) {
		_tprintf_s(_T("[ERRO] - Não foi possível abrir o arquivo de memória partilhada (%d).\n"), GetLastError());
		CloseHandle(sharedThread.hEvent);
		CloseHandle(sharedThread.hMutex);
		CloseHandle(hPipe);
		return -1;
    }

	sharedThread.continuar = TRUE;

	// criar thread para esperar atualizações da memória partilhada
    hThreadMemData = CreateThread(NULL, 0, EsperaMemData, &sharedThread, 0, NULL);
    if (hThreadMemData == NULL) {
        _tprintf(TEXT("[ERRO] - Criar ThreadMemData! (CreateThread)\n"));
		CloseHandle(sharedThread.hMapFile);
		CloseHandle(sharedThread.hEvent);
		CloseHandle(sharedThread.hMutex);
        CloseHandle(hPipe);
        return -1;
    }

    /*DADOS PARTILHADOS ENTRE THREAD PRINCIPAL E THREAD ESCUTA ARBITRO*/
    DadosPartilhados dadosPartilhados;
    dadosPartilhados.hMutex = &hMutex;
    dadosPartilhados.hEventoParar = CreateEvent(NULL, TRUE,FALSE, NULL);
    dadosPartilhados.hEventoAvancar = CreateEvent(NULL, TRUE, TRUE, NULL);
    dadosPartilhados.pontuacao = 0;
    dadosPartilhados.hPipe = &hPipe;
    dadosPartilhados.comandos = &comandos_jogador;
    dadosPartilhados.jogador = &jogador;
    dadosPartilhados.Continua=&continua;
    dadosPartilhados.jogoIniciado = FALSE;
    

    /*DADOS PARTILHADOS ENTRE THREAD ESCUTA ARBITRO*/
    threadescuta.Continua = &continua;
    threadescuta.header = &header;
    threadescuta.dadosPartilhados = &dadosPartilhados;

    ThreadArbitro = CreateThread(NULL, 0, threadArbitro, &threadescuta, 0, NULL);
    if (ThreadArbitro == NULL) {
        _tprintf(TEXT("[ERRO] - Criar ThreadInterface! (CreateThread)\n"));
        CloseHandle(sharedThread.hMapFile);
        CloseHandle(sharedThread.hEvent);
        CloseHandle(sharedThread.hMutex);
        CloseHandle(hPipe);
        sharedThread.continuar = FALSE;
        WaitForSingleObject(hThreadMemData, INFINITE);
		CloseHandle(hThreadMemData);
        return -1;
    }

    do {
        verifica_comandos(&dadosPartilhados);
    } while (continua);

    
	sharedThread.continuar = FALSE;

    WaitForSingleObject(ThreadArbitro, INFINITE);
    CloseHandle(ThreadArbitro);
	WaitForSingleObject(hThreadMemData, INFINITE);
	CloseHandle(hThreadMemData);
    CloseHandle(sharedThread.hMapFile);
    CloseHandle(sharedThread.hEvent);
    CloseHandle(sharedThread.hMutex);
    CloseHandle(hMutex);
    CloseHandle(hPipe);


    return 0;
}
