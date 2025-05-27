#ifndef STRUCTS_H
#define STRUCTS_H

#include <windows.h>
#include <tchar.h>

#define MAX_VISIBLE_LETRAS 100

typedef struct {
	HANDLE hEvent;
	HANDLE hMutex;
	HANDLE hMapFile;
}MEMDATA;

typedef struct {
	HANDLE hInstancia;
	OVERLAPPED overlap;
	BOOL activo;
}DadosPipe;

typedef struct {
	TCHAR letras_visiveis[MAX_VISIBLE_LETRAS];
	TCHAR palavra[100];
} SHAREDMEM_LETRAS;

typedef struct {
	TCHAR comando[256];
	int tipo_comando;
}Comandos_Jogador;

typedef struct {
	TCHAR username[MAX];
	float pontuacao;
	TCHAR* palavra;
	BOOL ativo;
}Jogador;

typedef struct {
	int max_letras;
	int ritmo;
	CRITICAL_SECTION csConfig;
} ConfigJogo;

typedef struct {
	DadosPipe* hPipes;
	HANDLE* hEvents;
	HANDLE hMutex;
	MEMDATA *memdata;
	int terminar;
	int nJogadores;
	int JogadorIndex;
	int * JogadorIndexLider;
	Jogador jogadores[DEFAULT_MAX_JOGADORES];
	ConfigJogo* config;
}ThreadDados;

typedef struct {
	TCHAR* letrasAtuais;
	TCHAR* dicionario;
	CRITICAL_SECTION cs; 
} Letters;

typedef struct {
	TCHAR* alfabeto;
	TCHAR* vogais;
	BOOLEAN continuar;
	ConfigJogo* config;
	Letters* letters;
	MEMDATA* memdata;
	SHAREDMEM_LETRAS* pSharedData;
} ThreadNewLet;

typedef struct {
	int jogadorIndex;
	ThreadDados* dados;
	Letters* letters;
	SHAREDMEM_LETRAS *pSharedData;
} ThreadParams;

typedef struct {
	DWORD tipo;
	DWORD tamanho;
} MensagemHeader;

typedef struct {
	Jogador* jogadores;
	int nJogadoresativos;
}EnviaDados;
#endif