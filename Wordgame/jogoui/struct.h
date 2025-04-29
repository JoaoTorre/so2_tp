#pragma once
#include <tchar.h>
#include <stdlib.h>
#include <Windows.h>

#define MAX 255


typedef struct {
	TCHAR username[MAX];
}Jogador;


typedef struct {
	TCHAR INICIO[MAX];
	float PONTUACAO;
}Respostas;

typedef struct {
	TCHAR comando[MAX];
	int tipo_comando;
} Comandos_Jogador;
