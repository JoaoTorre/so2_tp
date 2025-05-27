#pragma once 
#ifndef STRUCTS_H
#define STRUCTS_H

#include <windows.h>

#define MAX_VISIBLE_LETRAS 13
#define MAX 255

typedef struct {
	TCHAR username[MAX];
	float pontuacao;
	TCHAR* palavra;
	BOOL ativo;
	BOOL bot;
}Jogador;


typedef struct {
	DWORD tipo;
	DWORD tamanho;
} MensagemHeader;


typedef struct {
	TCHAR comando[MAX];
	int tipo_comando;
} Comandos_Jogador;

typedef struct {
	Jogador* jogadores;
	int nJogadoresativos;
}EnviaDados;

typedef struct {
	HANDLE* hPipe;
	HANDLE hEventoParar;
	HANDLE hEventoAvancar;
	HANDLE* hMutex;
	float pontuacao;
	Comandos_Jogador* comandos;
	Jogador* jogador;
	BOOL* Continua;
} DadosPartilhados;

typedef struct {
	BOOL* Continua;
	MensagemHeader* header;
	DadosPartilhados* dadosPartilhados;
} ThreadEscutaParam;

typedef struct {
	TCHAR letras_visiveis[MAX_VISIBLE_LETRAS];
	TCHAR palavra[MAX_VISIBLE_LETRAS];
} SHAREDMEM_LETRAS;

typedef struct {
	SHAREDMEM_LETRAS* pSharedData;
	HANDLE hMapFile;
	HANDLE hEvent;
	HANDLE hMutex;
	BOOL continuar;
	SHAREDMEM_LETRAS sharedDataCopy;
	CRITICAL_SECTION cs;
} SHARED_THREAD;

#endif STRUCTS_H
