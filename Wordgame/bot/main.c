#include "utils.h"
#include "struct.h"

DWORD WINAPI EsperaMemData(LPVOID param) {
	SHARED_THREAD* sharedThread = (SHARED_THREAD*)param;
	HANDLE handles[2] = { sharedThread->hEvent, sharedThread->hMutex };
	DWORD dwWaitResult;

	do {

		dwWaitResult = WaitForMultipleObjects(2, handles, FALSE, INFINITE);

		if (dwWaitResult == WAIT_FAILED) {
			_tprintf(_T("[ERRO <BOT> ] - WaitForMultipleObjects falhou com erro: %d\n"), GetLastError());
			return -1;
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
				_tprintf(_T("[ERRO <BOT> ] - Não foi possível mapear a visão do arquivo (%d).\n"), GetLastError());
				return -1;
			}

			EnterCriticalSection(&sharedThread->cs);
			//Copiar a estrutura para uma variavel local
			CopyMemory(&sharedThread->sharedDataCopy, sharedThread->pSharedData, sizeof(SHAREDMEM_LETRAS));
			LeaveCriticalSection(&sharedThread->cs);

			UnmapViewOfFile(sharedThread->pSharedData);
			ResetEvent(sharedThread->hEvent);
			ReleaseMutex(sharedThread->hMutex);
		}
		else if (dwWaitResult == WAIT_OBJECT_0 + 1) {

			ReleaseMutex(sharedThread->hMutex);
		}
		else {
			_tprintf(_T("[ERRO <BOT> ] - WaitForMultipleObjects retornou um resultado inesperado.\n"));
			sharedThread->continuar = FALSE;
			return -1;
		}

	} while (sharedThread->continuar);


}

DWORD WINAPI ThreadEscuta(LPVOID param) {
	ThreadEscutaParam* dados = (ThreadEscutaParam*)param;
	DWORD n;
	BOOL loginEnviado = FALSE;
	DadosPartilhados* dadosPartilhados;
	dadosPartilhados = dados->dadosPartilhados;
	Comandos_Jogador comandos;
	TCHAR resposta[255];

	if (!loginEnviado) {
		WaitForSingleObject(*dados->dadosPartilhados->hMutex, INFINITE);
		dados->dadosPartilhados->comandos->tipo_comando = 1;
		ReleaseMutex(*dados->dadosPartilhados->hMutex);
		WriteFile(*dados->dadosPartilhados->hPipe, dados->dadosPartilhados->comandos, sizeof(Comandos_Jogador), &n, NULL);
		WriteFile(*dados->dadosPartilhados->hPipe, dados->dadosPartilhados->jogador, sizeof(Jogador), &n, NULL);
		loginEnviado = TRUE;
		SetEvent(dadosPartilhados->hEventoAvancar);
	}

	while (*dados->Continua) {
		DWORD ret = WaitForMultipleObjects(2, (const HANDLE[]) { dados->dadosPartilhados->hEventoAvancar, dados->dadosPartilhados->hEventoParar }, FALSE, INFINITE);

		if (ret == WAIT_OBJECT_0) {
			BOOL ret = ReadFile(*dados->dadosPartilhados->hPipe, dados->header, sizeof(MensagemHeader), &n, NULL);
			if (!ret) {
				DWORD error = GetLastError();
				switch (error) {
				case ERROR_BROKEN_PIPE:
					*dados->Continua = FALSE;
					break;
				case ERROR_PIPE_NOT_CONNECTED:
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
						break;
					}
					case 99: {
						ReadFile(*dados->dadosPartilhados->hPipe, resposta, dados->header->tamanho, &n, NULL);
						resposta[n / sizeof(TCHAR)] = _T('\0');
						*dados->Continua = FALSE;
						break;
					}
					case 40: {
						ReadFile(*dados->dadosPartilhados->hPipe, resposta, dados->header->tamanho, &n, NULL);
						resposta[n / sizeof(TCHAR)] = _T('\0');
						*dados->Continua = FALSE;
						break;
					}
				}

			}
		}

	}
	_tprintf(TEXT("[BOT] - O jogo foi encerrado.\n"));
	*dados->Continua = FALSE;
	return 0;
}

int _tmain(int argc, LPTSTR argv[]){
	unsigned int wait_time;
	Jogador jogador;
	HANDLE hPipe, hThreadMemData, hThreadEscuta, hTimer;
    SHARED_THREAD sharedThread;
	BOOL continua = TRUE;
	Comandos_Jogador comandos_jogador;
	ThreadEscutaParam threadescuta;
	HANDLE hMutex;
	MensagemHeader header;

	TCHAR* dicionario[] = {
	_T("abacate"), _T("abelha"), _T("eu"), _T("acaso"), _T("adeus"), _T("agora"),
	_T("alegria"), _T("amado"), _T("amigo"), _T("andar"), _T("anjo"),
	_T("antes"), _T("arroz"), _T("astro"), _T("aviao"), _T("azul"),
	_T("baixo"), _T("balde"), _T("beijo"), _T("bicho"), _T("bola"),
	_T("bonito"), _T("brisa"), _T("cabra"), _T("caminho"), _T("carta"),
	_T("casal"), _T("casar"), _T("caso"), _T("caverna"), _T("cedo"),
	_T("chuva"), _T("claro"), _T("cobra"), _T("cor"), _T("cores"),
	_T("corpo"), _T("correr"), _T("costa"), _T("dado"), _T("dente"),
	_T("depois"), _T("deusa"), _T("dizer"), _T("dobra"), _T("doce"),
	_T("dor"), _T("duro"), _T("eco"), _T("eles"), _T("estar"),
	_T("etapa"), _T("exato"), _T("fada"), _T("falar"), _T("falta"),
	_T("fama"), _T("fazer"), _T("ferir"), _T("festa"), _T("fiel"),
	_T("fogo"), _T("folha"), _T("forca"), _T("fraco"), _T("fruta"),
	_T("fundo"), _T("garra"), _T("gato"), _T("gelo"), _T("gerar"),
	_T("girar"), _T("grito"), _T("honra"), _T("hora"), _T("homem"),
	_T("ideia"), _T("igual"), _T("ilha"), _T("inverno"), _T("irmao"),
	_T("jogar"), _T("junto"), _T("lento"), _T("livro"), _T("lobo"),
	_T("luz"), _T("mago"), _T("mamae"), _T("matar"), _T("mel"),
	_T("mundo"), _T("musica"), _T("nascer"), _T("neve"), _T("nuvem"),
	_T("novo"), _T("olhar"), _T("onda"), _T("ouro"), _T("paz"),
	_T("pente"), _T("plano"), _T("poder"), _T("praia"), _T("preto"),
	_T("rato"), _T("rosto"), _T("rua"), _T("saber"), _T("salto"),
	_T("ar"), _T("ele"), _T("ela"), _T("nos"), _T("lei"), NULL };

	DWORD n;
	TCHAR comando[13];
	TCHAR* letrasAtuais;
	TCHAR palavraCompleta[100] = _T("");

#ifdef UNICODE
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif

	if (argc != 3) {
		_tprintf(_T("[ERRO <BOT> ] - Número de argumentos não válido ./bot.exe <username> <tempo>\n"));
		return -1;
	}
	
	wcscpy_s(jogador.username, _countof(jogador.username), argv[1]);
	wait_time = _tstoi(argv[2]);
	//wcscpy_s(jogador.username, _countof(jogador.username), _T("olasydbsa"));
	//wait_time = 3;
	jogador.bot = TRUE;

	if (wait_time < 1) {
		_tprintf(_T("[ERRO <BOT> ] - Tempo de espera deve ser maior que 0.\n"));
		return -1;
	}

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
            _tprintf_s(_T("[ERRO <BOT> ] - Não foi possível abrir o pipe. GLE=%d\n"), GetLastError());
            return -1;
        }

        if (!WaitNamedPipe(name_pipe, 20000)) {
            _tprintf_s(_T("[ERRO <BOT> ] - Não foi possível abrir pipe: 20 segundos timed out\n"));
            return -1;
        }
	}

	/* dados para a memoria partilhada */
	sharedThread.hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MEMORIA_PARTILHADA_MUTEX);

	if (sharedThread.hMutex == NULL) {
		_tprintf_s(_T("[ERRO <BOT> ] - Não foi possível abrir o mutex de memória partilhada (%d).\n"), GetLastError());
		CloseHandle(hPipe);
		return -1;
	}

	sharedThread.hEvent = OpenEvent(EVENT_ALL_ACCESS, FALSE, MEMORIA_PARTILHADA_EVENTO);

	if (sharedThread.hEvent == NULL) {
		_tprintf_s(_T("[ERRO <BOT> ] - Não foi possível abrir o evento de memória partilhada (%d).\n"), GetLastError());
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
		_tprintf_s(_T("[ERRO <BOT> ] - Não foi possível abrir o arquivo de memória partilhada (%d).\n"), GetLastError());
		CloseHandle(sharedThread.hEvent);
		CloseHandle(sharedThread.hMutex);
		CloseHandle(hPipe);
		return -1;
	}

	sharedThread.continuar = TRUE;
	InitializeCriticalSection(&sharedThread.cs);

	hThreadMemData = CreateThread(NULL, 0, EsperaMemData, &sharedThread, 0, NULL);
	if (hThreadMemData == NULL) {
		_tprintf(TEXT("[ERRO <BOT> ] - Criar ThreadMemData! (CreateThread)\n"));
		CloseHandle(sharedThread.hMapFile);
		CloseHandle(sharedThread.hEvent);
		CloseHandle(sharedThread.hMutex);
		CloseHandle(hPipe);
		return -1;
	}

	/* COMUNICAÇÃO COM O ÁRBITRO */
	hMutex = CreateMutex(NULL, FALSE, NULL);

	if (hMutex == NULL) {
		printf("Erro ao criar mutex: %lu\n", GetLastError());
		return 1;
	}

	DadosPartilhados dadosPartilhados;
	dadosPartilhados.hMutex = &hMutex;
	dadosPartilhados.hEventoParar = CreateEvent(NULL, TRUE, FALSE, NULL);
	dadosPartilhados.hEventoAvancar = CreateEvent(NULL, TRUE, TRUE, NULL);
	if (dadosPartilhados.hEventoParar == NULL || dadosPartilhados.hEventoAvancar == NULL) {
		_tprintf(TEXT("[ERRO <BOT> ] - Não foi possível criar eventos de sincronização.\n"));
		CloseHandle(sharedThread.hMapFile);
		CloseHandle(sharedThread.hEvent);
		CloseHandle(sharedThread.hMutex);
		CloseHandle(hPipe);
		return -1;
	}

	dadosPartilhados.pontuacao = 0;
	dadosPartilhados.hPipe = &hPipe;
	dadosPartilhados.jogador = &jogador;
	dadosPartilhados.comandos = &comandos_jogador;
	dadosPartilhados.Continua = &continua;
	threadescuta.Continua = &continua;
	threadescuta.header = &header;
	threadescuta.dadosPartilhados = &dadosPartilhados;

	hThreadEscuta = CreateThread(NULL, 0, ThreadEscuta, &threadescuta, 0, NULL);
	if (hThreadEscuta == NULL) {
		_tprintf(TEXT("[ERRO <BOT> ] - Criar ThreadInterface! (CreateThread)\n"));
		CloseHandle(sharedThread.hMapFile);
		CloseHandle(sharedThread.hEvent);
		CloseHandle(sharedThread.hMutex);
		CloseHandle(hPipe);
		sharedThread.continuar = FALSE;
		WaitForSingleObject(hThreadMemData, INFINITE);
		CloseHandle(hThreadMemData);
		return -1;
	}

	hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	if (hTimer == NULL) {
		printf("Erro ao criar o timer: %lu\n", GetLastError());
		return 1;
	}

	// Tempo inicial de espera (relativo, em 100 nanossegundos)
	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -(LONGLONG)(wait_time * 10000000LL);

	// Define o timer para repetir a cada INTERVALO_SEGUNDOS
	if (!SetWaitableTimer(hTimer, &liDueTime, wait_time * 1000, NULL, NULL, FALSE)) {
		printf("Erro ao definir o timer: %lu\n", GetLastError());
		CloseHandle(hTimer);
		return 1;
	}

	do {
		WaitForSingleObject(hTimer, INFINITE);
		palavraCompleta[0] = _T('\0');

		EnterCriticalSection(&sharedThread.cs);
		letrasAtuais = sharedThread.sharedDataCopy.letras_visiveis;
		LeaveCriticalSection(&sharedThread.cs);

		//print letras atuais
		_tprintf(_T("[BOT] - Letras atuais: %s\n"), letrasAtuais);

		for (int i = 0; dicionario[i] != NULL; i++) {
			TCHAR temp[100];
			wcscpy_s(temp, _countof(temp), letrasAtuais);

			int podeFormar = 1;

			for (int j = 0; dicionario[i][j] != _T('\0'); j++) {
				TCHAR letra = dicionario[i][j];
				int encontrou = 0;

				for (int k = 0; temp[k] != _T('\0'); k++) {
					if (temp[k] == letra) {
						for (int m = k; temp[m] != _T('\0'); m++) {
							temp[m] = temp[m + 1];
						}
						encontrou = 1;
						break;
					}
				}

				if (!encontrou) {
					podeFormar = 0;
					break;
				}
			}

			if (podeFormar) {
				wcscpy_s(palavraCompleta, _countof(palavraCompleta), dicionario[i]);
				break; 
			}
		}

		if (_tcslen(palavraCompleta) > 0) {
			ResetEvent(dadosPartilhados.hEventoAvancar);
			SetEvent(dadosPartilhados.hEventoParar);
			WaitForSingleObject(hMutex, INFINITE);
			wcscpy_s(comandos_jogador.comando, _countof(comandos_jogador.comando), palavraCompleta);
			comandos_jogador.tipo_comando = 5;
			WriteFile(hPipe, &comandos_jogador, sizeof(Comandos_Jogador), &n, NULL);
			ReleaseMutex(hMutex);
			ResetEvent(dadosPartilhados.hEventoParar);
			SetEvent(dadosPartilhados.hEventoAvancar);
		}
		
	} while (continua);



	/* FECHAR HANDLES */
	sharedThread.continuar = FALSE;
	DeleteCriticalSection(&sharedThread.cs);
	CloseHandle(hTimer);
	WaitForSingleObject(hThreadMemData, INFINITE);
	CloseHandle(hThreadMemData);
	WaitForSingleObject(hThreadEscuta, INFINITE);
	CloseHandle(hThreadEscuta);
	CloseHandle(sharedThread.hMapFile);
	CloseHandle(sharedThread.hEvent);
	CloseHandle(sharedThread.hMutex);
}

