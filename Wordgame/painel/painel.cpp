// painel.cpp : Define o ponto de entrada para o aplicativo.
//


#include "struct.h"
#define MAX_LOADSTRING 100

// Variáveis Globais:                              
WCHAR szTitle[MAX_LOADSTRING];                  
WCHAR szWindowClass[MAX_LOADSTRING];   
HINSTANCE hInst;
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PAINEL, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PAINEL));

    MSG msg;


    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


//
//  FUNÇÃO: MyRegisterClass()
//
//  FINALIDADE: Registra a classe de janela.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAINEL));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PAINEL);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNÇÃO: InitInstance(HINSTANCE, int)
//
//   FINALIDADE: Salva o identificador de instância e cria a janela principal
//
//   COMENTÁRIOS:
//
//        Nesta função, o identificador de instâncias é salvo em uma variável global e
//        crie e exiba a janela do programa principal.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Armazenar o identificador de instância em nossa variável global

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


DWORD WINAPI EsperaAtualizacao(LPVOID lpParam) {
   SHARED_THREAD* dados = (SHARED_THREAD*)lpParam;

    while (1) {
        DWORD dw = WaitForSingleObject(dados->hEvent, INFINITE);
        if (dw == WAIT_OBJECT_0) {
            InvalidateRect(dados->hWnd, NULL, FALSE);
            UpdateWindow(dados->hWnd);
        }
        else {
            break; 
        }
    }

    return 0;
}

//
//  FUNÇÃO: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  FINALIDADE: Processa as mensagens para a janela principal.
//
//  WM_COMMAND  - processar o menu do aplicativo
//  WM_PAINT    - Pintar a janela principal
//  WM_DESTROY  - postar uma mensagem de saída e retornar
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{

    static HANDLE hMapFile = NULL;
    static SHAREDMEM_LETRAS* pSharedData = NULL;
    static HANDLE hEvento = NULL;
   
    switch (message)
    {

    case WM_CREATE:
    {

        hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, MEMORIA_PARTILHADA_NOME);
        if (hMapFile == NULL) {
            MessageBox(hWnd, _T("Não foi possível abrir memória partilhada."), _T("Erro"), MB_OK);
            break;
        }

        hEvento = OpenEvent(EVENT_MODIFY_STATE | SYNCHRONIZE, FALSE, MEMORIA_PARTILHADA_EVENTO);
        if (hEvento == NULL) {
            MessageBox(hWnd, _T("Erro ao abrir evento de sincronização."), _T("Erro"), MB_OK);
            CloseHandle(hMapFile);
            hMapFile = NULL;
            break;
        }

        pSharedData = (SHAREDMEM_LETRAS*)MapViewOfFile(hMapFile, FILE_MAP_READ, 0, 0, sizeof(SHAREDMEM_LETRAS));
        if (pSharedData == NULL) {
            MessageBox(hWnd, _T("Não foi possível mapear memória partilhada."), _T("Erro"), MB_OK);
            CloseHandle(hMapFile);
            hMapFile = NULL;
            break;
        }

        SHARED_THREAD* pSharedThread = (SHARED_THREAD*)malloc(sizeof(SHARED_THREAD));
        if (pSharedThread == NULL) {
            MessageBox(hWnd, _T("Falha ao alocar memória."), _T("Erro"), MB_OK);
            break;
        }

        pSharedThread->hEvent = &hEvento;
        pSharedThread->hWnd = hWnd;
        pSharedThread->pSharedData = pSharedData;

        CreateThread(NULL, 0, EsperaAtualizacao, pSharedThread, 0, NULL);
        break;

    }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            if (pSharedData) {
                TextOut(hdc, 10, 10, pSharedData->palavra, lstrlen(pSharedData->palavra));
            }
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        if (pSharedData) {
            UnmapViewOfFile(pSharedData);
            pSharedData = NULL;
        }
        if (hMapFile) {
            CloseHandle(hMapFile);
            hMapFile = NULL;
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}



