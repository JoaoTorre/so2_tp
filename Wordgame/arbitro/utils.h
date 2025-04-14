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


// caminho das keys
#define KEY_PATH _T("SOFTWARE\\TrabSO2")


// nome das keys
#define KEY_RITMO _T("RITMO")
#define KEY_MAXLETRAS _T("MAXLETRAS")

// CONSTANTES DEFAULT
#define MAX 256
#define BUFSIZE 512
#define DEFAULT_MAX_CLIENTES 20
#define DEFAULT_RITMO 3
#define DEFAULT_MAX_LETRAS 6
#define MAX_LETRAS 12

// CONSTANTES NOMES SEMAFOROS
#define SEMAPHORE_UNIQUE_ARBITRO_NAME _T("SEMAPHORE_UNIQUE_ARBITRO")


#endif UTILS_H