#pragma once
#include <tchar.h>
#include <stdlib.h>
#include <Windows.h>
#define NOME_MUTEX "Global\\MeuMutex"

#define MAX 255


typedef struct {
	TCHAR username[MAX];
	float PONTUACAO;
}Jogador;


typedef struct {
	TCHAR INICIO[MAX];
	Jogador jogador;
	DWORD nJogadores;
	Jogador ListaJogadores[20];
}Respostas;

typedef struct {
	TCHAR comando[MAX];
	int tipo_comando;
} Comandos_Jogador;


typedef struct {
	HANDLE *hPipe;            
	BOOL *Continua;
	Respostas respostas;
	Comandos_Jogador * comandos;
	HANDLE *hMutex;
	Jogador *jogador;
} ThreadEscutaParam;