 #include "utils.h"
#include "struct.h"



int troca_dados(void* estrutura_enviar, HANDLE hPipe, void* estrutura_comando, TCHAR* Resposta_Login) {
    DWORD ret, n;
    Comandos_Jogador* comandos_jogador = (Comandos_Jogador*)estrutura_comando;
    DWORD tamanho_estrutura = 0;

    // TIPO 1 - INICIO
    if (comandos_jogador->tipo_comando == 1) {
        Jogador* estrutura = (Jogador*)estrutura_enviar;
        tamanho_estrutura = sizeof(Jogador);
    }

    // Envio do tipo de comando
    ret = WriteFile(hPipe, comandos_jogador, sizeof(Comandos_Jogador), &n, NULL);
    if (!ret) {
        _tprintf(TEXT("[Erro] ao enviar identificador do tipo de comando: %d\n"), GetLastError());
        return FALSE;
    }

    // Envio da estrutura de dados 
    if (!WriteFile(hPipe, estrutura_enviar, tamanho_estrutura, &n, NULL)) {
        _tprintf(TEXT("[Erro] - Ao enviar mensagem do ARBITRO:: %d\n"), GetLastError());
        return FALSE;
    }

    // Recebendo a resposta
    ret = ReadFile(hPipe, Resposta_Login, sizeof(Resposta_Login), &n, NULL);

    if (!ret) {
        _tprintf(TEXT("[Erro] - Ao receber mensagem do ARBITRO: %d\n"), GetLastError());
        return FALSE;
    }

    return TRUE;
}



int _tmain(int argc, LPTSTR argv[]) {
    HANDLE hPipe;
    Jogador jogador;
    Comandos_Jogador comandos_jogador;
    BOOL continua = TRUE;
    TCHAR comando[MAX], ** comandoArray = NULL;
    DWORD nArgumentos = 0, n;
    unsigned int i = 0;
    BOOL ret = FALSE;
    TCHAR Resposta_Login[MAX];
    TCHAR username[MAX];

#ifdef UNICODE
    _setmode(_fileno(stderr), _O_WTEXT);
    _setmode(_fileno(stdin), _O_WTEXT);
    _setmode(_fileno(stdout), _O_WTEXT);
#endif

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

    _tprintf_s(_T("Insira nome de utilizador: "));
    _fgetts(username, MAX, stdin);

    if (_tcslen(username) > 0 && username[_tcslen(username) - 1] == _T('\n')) {
        username[_tcslen(username) - 1] = '\0';  
    }
    _tcscpy_s(jogador.username, MAX, username);
   
    //Envio de Indentificação Jogador
    comandos_jogador.comando[0] = _T("Login");
    comandos_jogador.tipo_comando = 1;

    if (troca_dados(&jogador, hPipe, &comandos_jogador,&Resposta_Login))
        _tprintf(TEXT("Resposta recebida: %s\n"), Resposta_Login);

    
    return 0;
}
