#pragma once
#include <tchar.h>
#include <stdlib.h>
#include <Windows.h>


#define MAX 255

typedef struct {
	TCHAR username[MAX];
	float pontuacao;
	BOOL ativo;
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
	HANDLE hMutex;
	float pontuacao;
	Comandos_Jogador* comandos;
	Jogador* jogador;
	BOOL Parar;
	BOOL* Continua;
} DadosPartilhados;

typedef struct { 
	BOOL *Continua;
	HANDLE *hMutex;
	HANDLE *hEvento;
	MensagemHeader *header;
	DadosPartilhados *dadosPartilhados;
} ThreadEscutaParam;

