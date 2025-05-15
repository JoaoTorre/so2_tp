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
	char letras_visiveis[MAX_VISIBLE_LETRAS];                   
} SHAREDMEM_LETRAS;

typedef struct {
	TCHAR comando[256];
	int tipo_comando;
}Comandos_Jogador;

typedef struct {
	TCHAR username[MAX];
	float pontuacao;
	BOOL ativo;
}Jogador;

typedef struct {
	int max_letras;
	int ritmo;
} ConfigJogo;

typedef struct {
	DadosPipe* hPipes;
	HANDLE* hEvents;
	HANDLE hMutex;
	MEMDATA memdata;
	int terminar;
	int nJogadores;
	int JogadorIndex;
	Jogador jogadores[DEFAULT_MAX_JOGADORES];
	ConfigJogo config;
}ThreadDados;

typedef struct {
	int jogadorIndex;
	ThreadDados* dados;
} ThreadParams;


typedef struct {
	DWORD tipo;
	DWORD tamanho;
} MensagemHeader;

typedef struct {
	Jogador* jogadores;
	int nJogadoresativos;
}EnviaDados;

typedef struct {
	TCHAR* letrasAtuais;
	const TCHAR* alfabeto;
	const TCHAR* vogais;
	BOOLEAN continuar;
	CRITICAL_SECTION cs;
	unsigned int ritmo;
	unsigned int max_letras;
} td_dataNewLetter;


#endif