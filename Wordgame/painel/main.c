#include "struct.h"

LRESULT CALLBACK trataEventos(HWND, UINT, WPARAM, LPARAM);

TCHAR szProgName[] = TEXT("Base");

int WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPTSTR lpCmdLine, int nCmdShow) {
    HWND hWnd;
    MSG lpMsg;
    WNDCLASSEX wcApp;

    #ifdef _UNICODE
    #define _tWinMain wWinMain
    #else
    #define _tWinMain WinMain
    #endif

    wcApp.cbSize = sizeof(WNDCLASSEX);
    wcApp.hInstance = hInst;
    wcApp.lpszClassName = szProgName;
    wcApp.lpfnWndProc = trataEventos;
    wcApp.style = CS_HREDRAW | CS_VREDRAW;
    wcApp.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcApp.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wcApp.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcApp.lpszMenuName = NULL;
    wcApp.cbClsExtra = 0;
    wcApp.cbWndExtra = 0;
    wcApp.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);

    if (!RegisterClassEx(&wcApp))
        return 0;

    hWnd = CreateWindow(
        szProgName,
        TEXT("Painel"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        HWND_DESKTOP,
        NULL,
        hInst,
        0);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);


    while (GetMessage(&lpMsg, NULL, 0, 0) > 0) {
        TranslateMessage(&lpMsg);
        DispatchMessage(&lpMsg);
    }

    return (int)lpMsg.wParam;
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
    free(dados);

    return 0;
}

LRESULT CALLBACK trataEventos(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
    static HANDLE hMapFile = NULL;
    static SHAREDMEM_LETRAS* pSharedData = NULL;
    static HANDLE hEvento = NULL;

    switch (messg) {

    case WM_CREATE: {
        hMapFile = OpenFileMapping(FILE_MAP_READ, FALSE, MEMORIA_PARTILHADA_NOME);
        if (hMapFile == NULL) {
            MessageBox(hWnd, _T("Não foi possível abrir memoria partilhada."), _T("Erro"), MB_OK);
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
            MessageBox(hWnd, _T("Não foi possível mapear memoria partilhada."), _T("Erro"), MB_OK);
            CloseHandle(hMapFile);
            hMapFile = NULL;
            break;
        }

        SHARED_THREAD* pSharedThread = (SHARED_THREAD*)malloc(sizeof(SHARED_THREAD));
        if (pSharedThread == NULL) {
            MessageBox(hWnd, _T("Falha ao alocar memória."), _T("Erro"), MB_OK);
            break;
        }

        pSharedThread->hEvent = hEvento;  
        pSharedThread->hWnd = hWnd;
        pSharedThread->pSharedData = pSharedData;

        CreateThread(NULL, 0, EsperaAtualizacao, pSharedThread, 0, NULL);
        break;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);

        int larguraRet = 390;
        int alturaRet = 90;
        int espacamento = 20;

        // Retângulo de cima
        int x1_top = (rcClient.right - larguraRet) / 2;
        int y1_top = (rcClient.bottom / 2) - alturaRet - (espacamento / 2);
        int x2_top = x1_top + larguraRet;
        int y2_top = y1_top + alturaRet;

        // Retângulo de baixo
        int x1_bottom = x1_top;
        int y1_bottom = (rcClient.bottom / 2) + (espacamento / 2);
        int x2_bottom = x1_bottom + larguraRet;
        int y2_bottom = y1_bottom + alturaRet;

        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
        HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);

        SelectObject(hdc, hPen);
        SelectObject(hdc, hBrush);

        Rectangle(hdc, x1_top, y1_top, x2_top, y2_top);
        Rectangle(hdc, x1_bottom, y1_bottom, x2_bottom, y2_bottom);

        if (pSharedData) {
            SIZE sz;

            // Texto no retângulo de cima (exemplo: palavra)
            GetTextExtentPoint32(hdc, pSharedData->palavra, lstrlen(pSharedData->palavra), &sz);
            int posX_top = x1_top + (larguraRet - sz.cx) / 2;
            int posY_top = y1_top + (alturaRet - sz.cy) / 2;
            TextOut(hdc, posX_top, posY_top, pSharedData->palavra, lstrlen(pSharedData->palavra));

            // Texto no retângulo de baixo (exemplo: letras_visiveis)
            GetTextExtentPoint32(hdc, pSharedData->letras_visiveis, lstrlen(pSharedData->letras_visiveis), &sz);
            int posX_bottom = x1_bottom + (larguraRet - sz.cx) / 2;
            int posY_bottom = y1_bottom + (alturaRet - sz.cy) / 2;
            TextOut(hdc, posX_bottom, posY_bottom, pSharedData->letras_visiveis, lstrlen(pSharedData->letras_visiveis));
        }

        DeleteObject(hPen);
        EndPaint(hWnd, &ps);
        break;
    }

    case WM_DESTROY:
        if (pSharedData) {
            UnmapViewOfFile(pSharedData);
            pSharedData = NULL;
        }

        if (hMapFile) {
            CloseHandle(hMapFile);
            hMapFile = NULL;
        }

        if (hEvento) {
            CloseHandle(hEvento);
            hEvento = NULL;
        }

        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, messg, wParam, lParam);
    }
    return 0;
}
