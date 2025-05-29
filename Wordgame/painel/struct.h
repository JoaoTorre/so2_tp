#ifndef STRUCTS_H
#define STRUCTS_H

#include <windows.h>
#include <tchar.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <strsafe.h>
#include <synchapi.h>

#define MAX_VISIBLE_LETRAS 100
#define MAX 256
#define DEFAULT_MAX_JOGADORES 20
#define MEMORIA_PARTILHADA_NOME _T("Global\FileMapping")
#define MEMORIA_PARTILHADA_EVENTO _T("Global\FileMapingEvent")



typedef struct {
	TCHAR username[MAX];
	float pontuacao;
	TCHAR* palavra;
	BOOL ativo;
	BOOL bot;
} Jogador;

typedef struct {
	TCHAR letras_visiveis[MAX_VISIBLE_LETRAS];
	TCHAR palavra[MAX_VISIBLE_LETRAS];
	Jogador jogadores[DEFAULT_MAX_JOGADORES];
} SHAREDMEM_LETRAS;


typedef struct {
	SHAREDMEM_LETRAS* pSharedData;
	HANDLE* hEvent;
	HWND hWnd;
} SHARED_THREAD;

#endif