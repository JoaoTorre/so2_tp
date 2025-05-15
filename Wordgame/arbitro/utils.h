#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <windows.h>
#include <tchar.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <strsafe.h>
#include <synchapi.h>

// CONSTANTES MEMORIA PARTILHADA
#define MEMORIA_PARTILHADA_NOME _T("Global\FileMapping")
#define MEMORIA_PARTILHADA_EVENTO _T("Global\FileMapingEvent")
#define MEMORIA_PARTILHADA_MUTEX _T("Global\FileMapingMutex")


// CONSTANTES PIPES
#define NAMEDPIPE_JOGADORES _T("\\\\.\\pipe\\mynamedpipe")


// caminho das keys
#define KEY_PATH _T("SOFTWARE\\TrabSO2")


// nome das keys
#define KEY_RITMO _T("RITMO")
#define KEY_MAXLETRAS _T("MAXLETRAS")

// CONSTANTES DEFAULT
#define MAX 256
#define BUFSIZE 512
#define DEFAULT_MAX_JOGADORES 20
#define DEFAULT_RITMO 3
#define DEFAULT_MAXLETRAS 6
#define MAXIMO_LETRAS 12


// CONSTANTES NOMES SEMAFOROS
#define SEMAPHORE_UNIQUE_ARBITRO_NAME _T("SEMAPHORE_UNIQUE_ARBITRO")

INT RandomNumber(unsigned int min, unsigned int max);
TCHAR* toUpperString(TCHAR* string);
TCHAR** splitString(TCHAR* str, const TCHAR* delim, unsigned int* size);
BOOL getValueFromKeyMAXLETRAS(unsigned int* maxLetras);
BOOL getValueFromKeyRITMO(unsigned int* nRitmo);
BOOL setValueToKeyRITMO(unsigned int nRitmo);
BOOL setValueToKeyMAXLETRAS(unsigned int maxLetras);
#endif UTILS_H