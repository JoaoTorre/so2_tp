#include "utils.h"
#include "struct.h"


int AdicionarJogador(ThreadDados* threadData, Jogador novoJogador, SHAREDMEM_LETRAS *pSharedData) {
    for (int i = 0; i < threadData->nJogadores; i++) {
        if (_tcscmp(threadData->jogadores[i].username, novoJogador.username) == 0 && threadData->hPipes[i].activo == TRUE) {
            return -1;
        }
    }
    WaitForSingleObject(threadData->hMutex, INFINITE);
    if (threadData->nJogadores < DEFAULT_MAX_JOGADORES) {
        threadData->jogadores[threadData->JogadorIndex] = novoJogador;  
        threadData->hPipes[threadData->JogadorIndex].activo;
        pSharedData->jogadores[threadData->JogadorIndex] = novoJogador;
    }
    ReleaseMutex(threadData->hMutex);
    return TRUE;
}

int VerificaNovoLider(ThreadDados* threadData) {
    int novoLider = -1;
    float maiorPontuacao = -1.0f;
    int contadorEmpate = 0;

    for (int i = 0; i < threadData->nJogadores; i++) {
        if (threadData->hPipes[i].activo == TRUE) {
            float pontos = threadData->jogadores[i].pontuacao;

            if (pontos > maiorPontuacao) {
                maiorPontuacao = pontos;
                novoLider = i;
                contadorEmpate = 1; 
            }
            else if (pontos == maiorPontuacao) {
                contadorEmpate++; 
            }
        }
    }

    if (contadorEmpate > 1) {
        return -1;
    }

    if (novoLider != -1 && novoLider != threadData->JogadorIndexLider) {
        threadData->JogadorIndexLider = novoLider;
        return novoLider;
    }

    return -1;
}


int BuscarNJogadoresAtivos(ThreadDados* threadData) {
    int nJogadoresAtivos = 0;

    for (int i = 0; i < threadData->nJogadores; i++) {
        if (threadData->hPipes[i].activo == TRUE) {
            nJogadoresAtivos++;
        }
    }
    return nJogadoresAtivos;
}

void AvisarJogadores(ThreadDados* threadData, Jogador jogador, TCHAR* tipo, HANDLE hpipe) {
    DWORD n;
    MensagemHeader header;

    for (int i = 0; i < threadData->nJogadores; i++) {
        if (threadData->hPipes[i].activo) {
            if (_tcscmp(tipo, _T("ENTROU")) == 0) {
                 if (threadData->hPipes[i].hInstancia != hpipe) {
                    TCHAR msg[] = _T("ENTROU");
                    header.tipo = 60;
                    header.tamanho = sizeof(jogador);
  
                    WriteFile(threadData->hPipes[i].hInstancia, &header, sizeof(header), &n, NULL);
                    WriteFile(threadData->hPipes[i].hInstancia, &jogador, sizeof(Jogador), &n, NULL);
                 }
                 else {
                     TCHAR msg[] = _T("ACEITE");
                     header.tipo = 98;
                     header.tamanho = sizeof(msg);
                     WriteFile(threadData->hPipes[i].hInstancia,&header, sizeof(header), &n, NULL);
                     WriteFile(threadData->hPipes[i].hInstancia,msg, sizeof(msg), &n, NULL);
                 }
             }

             if (_tcscmp(tipo, _T("SAIU")) == 0) {
                        TCHAR msg[] = _T("SAIU");
                        header.tipo = 50;
                        header.tamanho = sizeof(msg);

                        WriteFile(threadData->hPipes[i].hInstancia, &header, sizeof(header), &n, NULL);
                        WriteFile(threadData->hPipes[i].hInstancia, &jogador, sizeof(Jogador), &n, NULL);
             }   

             if (_tcscmp(tipo, _T("IJOGO")) == 0) {
                 TCHAR msg[] = _T("IJogo");
                 header.tipo = 10;
                 header.tamanho = sizeof(msg);

                 WriteFile(threadData->hPipes[i].hInstancia, &header, sizeof(header), &n, NULL);
                 WriteFile(threadData->hPipes[i].hInstancia, msg, sizeof(msg), &n, NULL);
             }

             if (_tcscmp(tipo, _T("FJOGO")) == 0) {
                 TCHAR msg[] = _T("FJogo");
                 header.tipo = 20;
                 header.tamanho = sizeof(msg);

                 WriteFile(threadData->hPipes[i].hInstancia, &header, sizeof(header), &n, NULL);
                 WriteFile(threadData->hPipes[i].hInstancia, msg, sizeof(msg), &n, NULL);
             }


             if (_tcscmp(tipo, _T("ACERTOU")) == 0) {
                 if (threadData->hPipes[i].hInstancia != hpipe) {
                     TCHAR msg[] = _T("ACERTOU");
                     header.tipo = 70;
                     header.tamanho = (_tcslen(jogador.palavra) + 1) * sizeof(TCHAR);
                     WriteFile(threadData->hPipes[i].hInstancia, &header, sizeof(header), &n, NULL);
                     WriteFile(threadData->hPipes[i].hInstancia, &jogador, sizeof(Jogador), &n, NULL);
                     WriteFile(threadData->hPipes[i].hInstancia, jogador.palavra, (_tcslen(jogador.palavra) + 1) * sizeof(TCHAR), &n, NULL);
                 }
             }


            if (_tcscmp(tipo, _T("AFRENTE")) == 0) {
                if (threadData->hPipes[i].hInstancia != hpipe) {
                    TCHAR msg[] = _T("AFRENTE");
                    header.tipo = 63;
                    header.tamanho = sizeof(jogador);
                }
                else {
                    TCHAR msg[] = _T("AFRENTE");
                    header.tipo = 64;
                    header.tamanho = sizeof(jogador);
                }

                WriteFile(threadData->hPipes[i].hInstancia, &header, sizeof(header), &n, NULL);
                WriteFile(threadData->hPipes[i].hInstancia, &jogador, sizeof(Jogador), &n, NULL);
            }
        }

    }
}

int ExcluirJogador(ThreadDados* threadData, TCHAR* username) {
   DWORD n;
   MensagemHeader header;
  
   for (int i = 0; i < threadData->nJogadores; i++) {
        RemoveNovaLinha(threadData->jogadores[i].username);
        if (_tcscmp(threadData->jogadores[i].username, username) == 0) {
            if (threadData->hPipes[i].activo) {
                WaitForSingleObject(threadData->hMutex, INFINITE);
                threadData->hPipes[i].activo = FALSE;
                ReleaseMutex(threadData->hMutex);
                TCHAR msg[] = _T("EXCLUIDO");
                header.tipo = 40;
                header.tamanho = sizeof(msg);
                WriteFile(threadData->hPipes[i].hInstancia, &header, sizeof(header), &n, NULL);
                WriteFile(threadData->hPipes[i].hInstancia, msg, sizeof(msg), &n, NULL);
                FlushFileBuffers(threadData->hPipes[i].hInstancia);
                DisconnectNamedPipe(threadData->hPipes[i].hInstancia);
                CloseHandle(threadData->hPipes[i].hInstancia);
            } 
            return TRUE;
        }

    }
   return FALSE;
}

void ImprimirJogadores(ThreadDados* threadData) {
     for (int i = 0; i < threadData->nJogadores; i++) {
       if(threadData->hPipes[i].activo)
            _tprintf(TEXT("Jogador %d: %s - Pontuação: %.2f\n"), i + 1, threadData->jogadores[i].username, threadData->jogadores[i].pontuacao);
    }
}

int VerificaLetras(TCHAR leitura[12], TCHAR* letrasAtuais, CRITICAL_SECTION cs) {
    EnterCriticalSection(&cs);
    int tamLetras = (int)_tcslen(letrasAtuais);
    BOOL letraValida;


    for (int i = 0; i < _tcslen(leitura); i++) {
        letraValida = FALSE;
        for (int j = 0; j < tamLetras; j++) {
          
            if (leitura[i] == letrasAtuais[j]) {
                letrasAtuais[j] = _T(' ');
                letraValida = TRUE;
                break;
            }
        }

        if (!letraValida) {
            _tprintf(_T("Letra %c não é válida!\n"), leitura[i]);
            LeaveCriticalSection(&cs);

            return 0;
        }
        _tprintf(_T("Letras VERIFICA\n"), leitura[i]);
    }

    LeaveCriticalSection(&cs);
    return 1;
}

int VerificaPalavra(TCHAR leitura[12], TCHAR* dicionario[]) {
    if (leitura == NULL || dicionario == NULL){
        _tprintf(_T("ERRO: dicionario ou leitura é NULL!\n"));
        return 0;
    }

    for (int i = 0; dicionario[i] != NULL; i++) {  
        if (_tcscmp(leitura, dicionario[i]) == 0) {

            _tprintf(_T("[ARBITRO] - Palavra ACERTADA: %ls -> %ls\n"), leitura, dicionario[i]);
            return 1;  
        }
    }
    return 0;  
}

VOID OrdenarArray(TCHAR* letras) {
    int j = 0;

    for (int i = 0; i <= (int) _tcslen(letras); i++) {
        if (letras[i] != _T(' ')) {
            letras[j++] = letras[i];
        }
    }

    while (j <= (int) _tcslen(letras)) {
        letras[j++] = _T(' ');
    }
}

VOID novaLetra(TCHAR* letrasAtuais, TCHAR alfabeto[], TCHAR vogais[], unsigned int maxLetras) {
    int controladorVogais = 0;
    TCHAR letraNova = _T('\0');
    int len = (int)_tcslen(letrasAtuais);

    if (len > 3) {
        for (int i = 0; i < len; i++) {
            if (letrasAtuais[i] == _T('a') || letrasAtuais[i] == _T('e') ||
                letrasAtuais[i] == _T('i') || letrasAtuais[i] == _T('o') ||
                letrasAtuais[i] == _T('u')) {
                controladorVogais++;
            }
        }
        if (controladorVogais < 3) {
            letraNova = vogais[RandomNumber(0, 4)];
        }
        else {
            letraNova = alfabeto[RandomNumber(0, 22)];
        }
    }
    else {
        letraNova = alfabeto[RandomNumber(0, 22)];
    }

    if (len < maxLetras) {
        letrasAtuais[len] = letraNova;
        letrasAtuais[len + 1] = _T('\0');
        return;
    }

    // Se o array estiver cheio
    for (int i = 0; i < maxLetras - 1; i++) {
        letrasAtuais[i] = letrasAtuais[i + 1];
    }
    letrasAtuais[maxLetras - 1] = letraNova;
}



void atualizarLetrasDaMemoriaPartilhada(ThreadNewLet* data) {
    TCHAR letraVisiveis[250];
  
    WaitForSingleObject(data->memdata->hMutex, INFINITE);
        _tcscpy_s(letraVisiveis, MAX_VISIBLE_LETRAS, data->letters->letrasAtuais);
        _tcscpy_s(data->pSharedData->letras_visiveis, MAX_VISIBLE_LETRAS, letraVisiveis);
    ReleaseMutex(data->memdata->hMutex);
    SetEvent(data->memdata->hEvent);
  
}

void ShowActualLetters(TCHAR* letrasAtuais) {
   _tprintf_s(_T("\nLetras atuais: ")); 

    for (int i = 0; i < (int)_tcslen(letrasAtuais); i++) {
        _tprintf(_T("%c "), letrasAtuais[i]);
    }
 
}

int VerificaPontuacao(TCHAR* leitura,Jogador *jogador,Letters* letters,ConfigJogo* config,HANDLE* hMutex,HANDLE* hEvent,SHAREDMEM_LETRAS* pSharedData,ThreadNewLet* threadNewLet) {
    TCHAR letrasAtuaisCopyVerificacao[13];
    size_t len = _tcslen(leitura);
    int resultado;

    if (len > 0 && leitura[len - 1] == _T('\n')) {
        leitura[len - 1] = _T('\0');
        len--;
    }

    if (len < 1 || len > MAXIMO_LETRAS) {
        _tprintf_s(_T("Palavra muito longa ou vazia!\n"));
        jogador->pontuacao -= 0.5f * len;
        return jogador->pontuacao;
    }

    EnterCriticalSection(&letters->cs);
    _tcsncpy_s(letrasAtuaisCopyVerificacao, _countof(letrasAtuaisCopyVerificacao), letters->letrasAtuais, _TRUNCATE);
    LeaveCriticalSection(&letters->cs);
    _tprintf(_T("[ARBITRO] - Palavra recebida: %ls -> %ls\n"), leitura, jogador->username);
    if (VerificaLetras(leitura, letrasAtuaisCopyVerificacao, letters->cs)) {
 
        if (VerificaPalavra(leitura, letters->dicionario)) {

            _tprintf_s(_T("Palavra correta! Pontos atribuídos: %d\n"), len);
            _tprintf_s(_T("Letras atuais: %s\n"),letters->letrasAtuais);

            jogador->pontuacao += len;

            OrdenarArray(letrasAtuaisCopyVerificacao);

            EnterCriticalSection(&letters->cs);
            _tcsncpy_s(letters->letrasAtuais, config->max_letras + 1, letrasAtuaisCopyVerificacao, _TRUNCATE);
            atualizarLetrasDaMemoriaPartilhada(threadNewLet);
            LeaveCriticalSection(&letters->cs);

            WaitForSingleObject(hMutex, INFINITE);
            _tcscpy_s(pSharedData->palavra, MAX_VISIBLE_LETRAS, leitura);
            ReleaseMutex(hMutex);
            SetEvent(hEvent);
            resultado = 1;
        }
        else {
            _tprintf_s(_T("Palavra não encontrada!\n"));
            jogador->pontuacao -= 0.5f * len;
            resultado = 2;
        }
    }
    else {
        _tprintf_s(_T("Letras erradas! Tente novamente.\n"));
        jogador->pontuacao -= 0.5f * len;
        resultado = 2;
    }

    return resultado;
}


DWORD WINAPI ThreadNewLetter(LPVOID param) {
	ThreadNewLet* data = (ThreadNewLet*)param;

    HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);

    if (hTimer == NULL) {
        printf("[ERRO] - ao criar o timer: %lu\n", GetLastError());
        return -1;
    }

    while (data->continuar) {

        DWORD wait = WaitForMultipleObjects(2,(HANDLE[]) {*(data->hEventoInicio), *(data->hEventoFim)}, FALSE, INFINITE);

        if (wait == WAIT_OBJECT_0) {
            LARGE_INTEGER liDueTime;

            EnterCriticalSection(&data->config->csConfig);
            liDueTime.QuadPart = -(LONGLONG)(data->config->ritmo * 10000000LL);
            LeaveCriticalSection(&data->config->csConfig);

            if (!SetWaitableTimer(hTimer, &liDueTime, 0, NULL, NULL, FALSE)) {
                printf("Erro ao definir o timer: %lu\n", GetLastError());
                break;
            }

            WaitForSingleObject(hTimer, INFINITE);
            EnterCriticalSection(&data->letters->cs);
            novaLetra(data->letters->letrasAtuais, data->alfabeto, data->vogais, data->config->max_letras);
            atualizarLetrasDaMemoriaPartilhada(data);
            ShowActualLetters(data->letters->letrasAtuais);
            LeaveCriticalSection(&data->letters->cs);

        }
    }

    CloseHandle(hTimer);
    return 0;
}

DWORD WINAPI threadTrataCliente(LPVOID param) {
    ThreadParams* params = (ThreadParams*)param;
    BOOL ret, continua = TRUE;
    Comandos_Jogador comandos;
    Jogador jogador;
    DWORD n;
    MensagemHeader header;
    EnviaDados dadosParaEnviar;
    HANDLE pipe = params->dados->hPipes[params->jogadorIndex].hInstancia;

   do{
        // Ler comando
        ret = ReadFile(pipe, &comandos, sizeof(comandos), &n, NULL);
        if (!ret) {
            DWORD error = GetLastError();
            switch (error) {
            case ERROR_BROKEN_PIPE:
                _tprintf(TEXT("[ARBITRO] - O jogador %s desconectou-se do jogo"),jogador.username);
                params->dados->hPipes[params->jogadorIndex].activo = FALSE;
                AvisarJogadores(params->dados, jogador, _T("SAIU"), pipe);
                if (BuscarNJogadoresAtivos(params->dados) == 1) {
                    AvisarJogadores(params->dados, jogador, _T("FJOGO"), pipe);
                    ResetEvent(*(params->dados->hEventoInicio));
                    SetEvent(*(params->dados->hEventoFim));
                    _tprintf(TEXT("\n[ARBITRO]- Jogo encerrou\n"));
                }
                break;
            case ERROR_PIPE_NOT_CONNECTED:
                _tprintf(TEXT("[ARBITRO] - O jogador foi desconectado %s\n"),jogador.username);
                params->dados->hPipes[params->jogadorIndex].activo = FALSE;
                AvisarJogadores(params->dados, jogador, _T("SAIU"), pipe);
                if (BuscarNJogadoresAtivos(params->dados) == 1) {
                    AvisarJogadores(params->dados, jogador, _T("FJOGO"), pipe);
                    ResetEvent(*(params->dados->hEventoInicio));
                    SetEvent(*(params->dados->hEventoFim));
                    _tprintf(TEXT("\n[ARBITRO]- Jogo encerrou\n"));
                }
                break;
            default:
                _tprintf(TEXT("[ERRO] - Falha ao ler comando do pipe. Código de erro: %d\n"), error);
                break;
            }
            continua = FALSE;
            break;
        }

        switch (comandos.tipo_comando) {
        case 1: // INICIO LOGIN
          
            ret = ReadFile(pipe, &jogador, sizeof(Jogador), &n, NULL);
            if (!ret || n != sizeof(Jogador)) {
                _tprintf(TEXT("[ERRO] - Falha ao ler struct Jogador. Código: %d\n"), GetLastError());
                continua = FALSE;
                break;
            }

            _tprintf(TEXT("[ARBITRO] - Login recebido. Username: %s\n"), jogador.username);
            jogador.pontuacao = 0;
            params->dados->JogadorIndex = params->jogadorIndex;
            int inicio_jogo = FALSE;
         
            WaitForSingleObject(params->dados->hMutex, INFINITE);
            if (AdicionarJogador(params->dados, jogador,params->pSharedData) == -1) {
                _tprintf(TEXT("[ARBITRO] - Jogador já existe: %s\n"), jogador.username);

                // "NACEITE"
                TCHAR msg[] = _T("NACEITE");
                header.tipo = 99;
                header.tamanho = sizeof(msg);
                WriteFile(pipe, &header, sizeof(header), &n, NULL);
                WriteFile(pipe, msg, sizeof(msg), &n, NULL);
                params->dados->hPipes[params->jogadorIndex].activo = FALSE;
                continua = FALSE;
            }
            else { // "ACEITE"
                AvisarJogadores(params->dados, jogador, _T("ENTROU"), pipe);
                comandos.tipo_comando = 6;

                //AVISAR JOGADORES INICIO DE JOGO
                if (BuscarNJogadoresAtivos(params->dados) >= 2) {
                    AvisarJogadores(params->dados, jogador, _T("IJOGO"), pipe);                  
                    ReleaseMutex(params->dados->hMutex);
                    inicio_jogo = TRUE;
                    _tprintf(TEXT("\n[ARBITRO]- Jogo iniciado\n"));
                }
            }
            ReleaseMutex(params->dados->hMutex);

            if (inicio_jogo) {
                ResetEvent(*(params->dados->hEventoFim));
                SetEvent(*(params->dados->hEventoInicio));
                SetEvent(params->dados->memdata->hEvent); 
            }
            break;

        case 2: // PEDIDO PONTUAÇÃO
        {
            header.tipo = 2;
            header.tamanho = sizeof(float);
            WriteFile(pipe, &header, sizeof(header), &n, NULL);

            float pontuacao = params->dados->jogadores[params->jogadorIndex].pontuacao;
            WriteFile(pipe, &pontuacao, sizeof(float), &n, NULL);
            break;
        }

        case 3: // PEDIDO LISTA DE JOGADORES
        {
            ZeroMemory(&dadosParaEnviar, sizeof(EnviaDados));
            dadosParaEnviar.jogadores = (Jogador*)malloc(DEFAULT_MAX_JOGADORES * sizeof(Jogador));

            header.tipo = 3;
            header.tamanho = sizeof(EnviaDados);
            WriteFile(pipe, &header, sizeof(header), &n, NULL);


            for (int i = 0; i < params->dados->nJogadores; i++) {
                if (params->dados->hPipes[i].activo) {
                    dadosParaEnviar.jogadores[dadosParaEnviar.nJogadoresativos] = params->dados->jogadores[i];
                    dadosParaEnviar.nJogadoresativos++;
                }
            }

            WriteFile(pipe, &dadosParaEnviar.nJogadoresativos, sizeof(int), &n, NULL);
            WriteFile(pipe, dadosParaEnviar.jogadores, dadosParaEnviar.nJogadoresativos * sizeof(Jogador), &n, NULL);

            free(dadosParaEnviar.jogadores);
            break;
        }

        case 4: // JOGADOR SAIU
            _tprintf(TEXT("[ARBITRO] - Jogador saiu: %s\n"), jogador.username); 
            params->dados->hPipes[params->jogadorIndex].activo = FALSE;
            AvisarJogadores(params->dados, jogador, _T("SAIU"), pipe); 
            if (BuscarNJogadoresAtivos(params->dados) == 1) {
                AvisarJogadores(params->dados, jogador, _T("FJOGO"), pipe);
                ResetEvent(*(params->dados->hEventoInicio));
                SetEvent(*(params->dados->hEventoFim));
                comandos.tipo_comando = 6;
            }
            FlushFileBuffers(pipe);
            DisconnectNamedPipe(pipe);
            continua = FALSE;
            break;
        case 5: //RECEBE PALAVRA 
            header.tipo = 2;
            header.tamanho = sizeof(float);
            int resultado = 0;
            DWORD tamanho_palavra;
            WriteFile(pipe, &header, sizeof(header), &n, NULL);
     
            WaitForSingleObject(params->dados->hMutex, INFINITE);
                Jogador* jogadorAtual = &params->dados->jogadores[params->jogadorIndex];
                resultado =VerificaPontuacao(comandos.comando,jogadorAtual,params->letters,params->dados->config,params->dados->memdata->hMutex,params->dados->memdata->hEvent,params->pSharedData,params->threadNewLet);
                jogador.palavra = (TCHAR*)malloc((_tcslen(comandos.comando) + 1) * sizeof(TCHAR));
                jogador.pontuacao = params->dados->jogadores[params->jogadorIndex].pontuacao;
                tamanho_palavra = (_tcslen(jogador.palavra) + 1) * sizeof(TCHAR);
                _tcscpy_s(jogador.palavra, _tcslen(comandos.comando) + 1, comandos.comando);

                if (resultado == 1) {
                     WriteFile(pipe,&resultado, sizeof(int), &n, NULL);
                     WriteFile(pipe, &params->dados->jogadores[params->jogadorIndex].pontuacao, sizeof(float), &n, NULL);
                     WriteFile(pipe,&tamanho_palavra,sizeof(DWORD), &n, NULL);
                     WriteFile(pipe,jogador.palavra, (_tcslen(jogador.palavra) + 1) * sizeof(TCHAR), &n, NULL);
                     AvisarJogadores(params->dados,jogador, _T("ACERTOU"), pipe);
                     comandos.tipo_comando = 6;
                 }

                if (resultado == 2) {
                    WriteFile(pipe, &resultado, sizeof(int), &n, NULL);
                    WriteFile(pipe, &tamanho_palavra, sizeof(DWORD), &n, NULL);
                    WriteFile(pipe, jogador.palavra, (_tcslen(jogador.palavra) + 1) * sizeof(TCHAR), &n, NULL);
                }
                params->pSharedData->jogadores[params->jogadorIndex].pontuacao = jogador.pontuacao;
            ReleaseMutex(params->dados->hMutex);

            SetEvent(params->dados->memdata->hEvent);

            int jogadorFrente = VerificaNovoLider(params->dados);

            // ESTÁ A FRENTE
            if (jogadorFrente == params->jogadorIndex) {
                AvisarJogadores(params->dados,jogador, _T("AFRENTE"), pipe);
            }
            else if (jogadorFrente == -1) {
               // EMPATE COM ALGUM JOGADOR OU NÃO EXISTE MUDANÇA DE LIDER
            }
            // OUTRO JOGADOR PASSOU A SER LIDER
            else if(jogadorFrente != params->jogadorIndex){
                AvisarJogadores(params->dados,params->dados->jogadores[jogadorFrente], _T("AFRENTE"), params->dados->hPipes[jogadorFrente].hInstancia);
            }

            break;

        case 6:
            continue;
            break;
        default:
            _tprintf(TEXT("[ARBITRO] - Comando desconhecido: %d\n"), comandos.tipo_comando);
            continua = FALSE;
            break;

          }
            
        }while (continua);
    return 0;
   
}

DWORD WINAPI threadInterface(LPVOID param) {
    ThreadDados *dados = (ThreadDados*)param;
    TCHAR comando[MAX], ** comandoArray = NULL;
    TCHAR nomeFicheiro[MAX];
    DWORD nArgumentos = 0;
    BOOL continua = TRUE;
    TCHAR username [MAX];
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    TCHAR cmdLine[256];
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));


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
                        _tprintf(_T("\nJogador excluido:%s\n"), comandoArray[1]);

                }
                else if (_tcscmp(comandoArray[0], _T("iniciarbot")) == 0 && nArgumentos == 3) {
                    
                  
                    _stprintf_s(cmdLine, _countof(cmdLine), _T("bot.exe %s %s"), comandoArray[1], comandoArray[2]);


                    if (!CreateProcess(
                        NULL,        // Nome do módulo (NULL usa o nome no cmdLine)
                        cmdLine,     // Linha de comando (pode ser modificada)
                        NULL,        // Atributos de segurança do processo
                        NULL,        // Atributos de segurança da thread
                        FALSE,       // Herda descritores?
                        0,           // Flags de criação
                        NULL,        // Ambiente (NULL = herda)
                        NULL,        // Diretório atual
                        &si,         // Info de startup
                        &pi))        // Info do processo (recebe os handles)
                    {
                        _tprintf(_T("Erro ao criar processo (%d).\n"), GetLastError());
                        return 1;
                    }

                }
                else if (_tcscmp(comandoArray[0], _T("acelerar")) == 0) {
					EnterCriticalSection(&dados->config->csConfig);
                    if (dados->config->ritmo > 1) {
                        dados->config->ritmo--;
                        _tprintf(_T("\nRitmo -> %d\n"), dados->config->ritmo);
                    }
                    else {
						dados->config->ritmo = 1;
                        _tprintf(_T("\nRitmo -> %d (Máximo ritmo)\n"), dados->config->ritmo);
                    }
                    LeaveCriticalSection(&dados->config->csConfig);
                }
                else if (_tcscmp(comandoArray[0], _T("travar")) == 0) {
					EnterCriticalSection(&dados->config->csConfig);
					dados->config->ritmo++;
                    LeaveCriticalSection(&dados->config->csConfig);
                    _tprintf(_T("\nRitmo -> %d\n"), dados->config->ritmo);
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

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    free(comandoArray);
    SetEvent(dados->hEvents[0]);
    dados->terminar = TRUE;
    return 0;
}

int _tmain(int argc, TCHAR* argv[]) {
    HANDLE hSemaphoreArbitro, hSemaphoreMaxClientes,hEventTodos,hPipe,hEventoInicio,hEventoFim,hThreadInterface, hThreadNewLetter;
    HANDLE* hThreads;
    ThreadDados dados;
    MEMDATA memdata;
    DWORD offset, nBytes;
    int i;
	Letters letters;
    ConfigJogo dadosConfig;
	ThreadNewLet threadNewLet;

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
    InitializeCriticalSection(&dadosConfig.csConfig);

    if (!getValueFromKeyMAXLETRAS(&dadosConfig.max_letras)) {
        _ftprintf(stderr, _T("[ERRO] - Não foi possível obter o valor de MAXLETRAS"));
        dadosConfig.max_letras = DEFAULT_MAXLETRAS;
        // guardar numa key o valor default
		setValueToKeyMAXLETRAS(DEFAULT_MAXLETRAS);
    }
    else if (dadosConfig.max_letras > MAXIMO_LETRAS) {
		_ftprintf(stderr, _T("[ERRO] - O valor de MAXLETRAS não pode ser superior a %d"), MAXIMO_LETRAS);
        dadosConfig.max_letras = MAXIMO_LETRAS;
		setValueToKeyMAXLETRAS(MAXIMO_LETRAS);
    }

    if (!getValueFromKeyRITMO(&dadosConfig.ritmo)) {
		_ftprintf(stderr, _T("[ERRO] - Não foi possível obter o valor de RITMO"));
        dadosConfig.ritmo = DEFAULT_RITMO;
		setValueToKeyRITMO(DEFAULT_RITMO);
    }else if(dadosConfig.ritmo < 1){
		_ftprintf(stderr, _T("[ERRO] - O valor de RITMO não pode ser inferior a 1"));
        dadosConfig.ritmo = DEFAULT_RITMO;
		setValueToKeyRITMO(DEFAULT_RITMO);
	}
    dados.nJogadores = 0;

    // Preencher estrutura para letras
    TCHAR buffer[12] = _T("eu");
    letters.letrasAtuais = buffer;
    TCHAR* dicionario[] = {
    _T("abacate"), _T("abelha"), _T("acaso"), _T("adeus"), _T("agora"),
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
       _T("eu"), _T("ar"), _T("ele"), _T("ela"), _T("nos"), _T("lei"), _T("a"), _T("e"), _T("i"), NULL};
    letters.dicionario = dicionario;
    InitializeCriticalSection(&letters.cs);

    

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


     hEventoInicio = CreateEvent(NULL, TRUE, FALSE, NULL);
     hEventoFim = CreateEvent(NULL, TRUE, TRUE, NULL);



     // Criar mutex para escrever na memória
     memdata.hMutex = CreateMutex(NULL, FALSE, MEMORIA_PARTILHADA_MUTEX);

     if (memdata.hMutex == NULL) {
         _tprintf_s(_T("[ERRO] - Não foi possível criar mutex para memória compartilhada (%d).\n"), GetLastError());
         CloseHandle(hSemaphoreArbitro);
         CloseHandle(memdata.hMapFile);
         CloseHandle(memdata.hEvent);
         return -1;
     }


     SHAREDMEM_LETRAS* pSharedData = (SHAREDMEM_LETRAS*)MapViewOfFile(
         memdata.hMapFile,
         FILE_MAP_WRITE, 
         0,
         0,
         sizeof(SHAREDMEM_LETRAS));

     if (pSharedData == NULL) {
         _tprintf_s(_T("[ERRO] - Não foi possível mapear visão da memória compartilhada (%d).\n"), GetLastError());
         CloseHandle(memdata.hMapFile);
         return -1;
     }


     /*        PIPES           */

     dados.hPipes = malloc(DEFAULT_MAX_JOGADORES * sizeof(DadosPipe)); // Aloca memória para os arrays hPipes
     dados.hEvents = malloc(DEFAULT_MAX_JOGADORES * sizeof(HANDLE)); // Aloca memória para os arrays hEvents
     hThreads = malloc(DEFAULT_MAX_JOGADORES * sizeof(HANDLE)); // Aloca memória para os arrays hThreads
     dados.JogadorIndex = 0; //Índice de Cliente no Array hPipes
     dados.terminar = 0; //Flag como sinal de  terminar
     dados.config = &dadosConfig;
     dados.memdata = &memdata;

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
         hEventTodos = CreateEvent(NULL, TRUE, FALSE, NULL);

         if (hEventTodos == NULL) {
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
             CloseHandle(hEventTodos);
             exit(-1);
         }


         ZeroMemory(&dados.hPipes[i].overlap, sizeof(dados.hPipes[i].overlap));
         dados.hPipes[i].hInstancia = hPipe;
         dados.hPipes[i].overlap.hEvent = hEventTodos;
         dados.hEvents[i] = hEventTodos;
         dados.hPipes[i].activo = FALSE;
         dados.JogadorIndexLider = -1;
         dados.hEventoInicio = &hEventoInicio;
         dados.hEventoFim = &hEventoFim;
       

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
    

     // Preencher estrutura para Thread que gera novas letras
     threadNewLet.alfabeto = _T("abcdefghijlmnopqrstuvxz");
     threadNewLet.vogais = _T("aeiou");
     threadNewLet.continuar = TRUE;
     threadNewLet.config = &dadosConfig;
     threadNewLet.letters = &letters;
     threadNewLet.memdata = &memdata;
     threadNewLet.pSharedData = pSharedData;
     threadNewLet.hEventoInicio = &hEventoInicio;
     threadNewLet.hEventoFim = &hEventoFim;


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

     // criar thread nova letra
	 hThreadNewLetter = CreateThread(NULL, 0, ThreadNewLetter, &threadNewLet, 0, NULL);

     _tprintf(TEXT("[ARBITRO] - Esperar ligacão de um Jogador/Bot...\n"));
     while (!dados.terminar && dados.nJogadores < DEFAULT_MAX_JOGADORES) {
         offset = WaitForMultipleObjects(DEFAULT_MAX_JOGADORES, dados.hEvents, FALSE, INFINITE);
         i = offset - WAIT_OBJECT_0;

         if (i >= 0 && i < DEFAULT_MAX_JOGADORES) {
             _tprintf(TEXT("[ARBITRO] - Jogador/Bot %d entrou...\n"), i+1);
             if (GetOverlappedResult(dados.hPipes[i].hInstancia,
                 &dados.hPipes[i].overlap, &nBytes, FALSE)) {

                 ResetEvent(dados.hEvents[i]);
                 WaitForSingleObject(dados.hMutex, INFINITE);
                 dados.hPipes[i].activo = TRUE;
                 ReleaseMutex(dados.hMutex);

                 ThreadParams* params = malloc(sizeof(ThreadParams));
                 params->jogadorIndex = i;
                 params->dados = &dados;
                 params->letters = &letters;
                 params->pSharedData = pSharedData;
				 params->threadNewLet = &threadNewLet;

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

	 DeleteCriticalSection(&letters.cs);
	 DeleteCriticalSection(&dadosConfig.csConfig);

     free(dados.hPipes);
     free(dados.hEvents);
     free(hThreads);

     CloseHandle(hThreadInterface);
     CloseHandle(hSemaphoreArbitro);  // Liberta semáforo
     CloseHandle(memdata.hMapFile);   // Liberta memória compartilhada 
     CloseHandle(memdata.hEvent);     // Liberta Evento
     CloseHandle(memdata.hMutex);     // Liberta mutex
	return 0;
}