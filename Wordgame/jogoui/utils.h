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

#define name_pipe  _T("\\\\.\\pipe\\mynamedpipe")

#define MAX_VISIBLE_LETRAS 13
#define MEMORIA_PARTILHADA_NOME _T("Global\FileMapping")
#define MEMORIA_PARTILHADA_EVENTO _T("Global\FileMapingEvent")
#define MEMORIA_PARTILHADA_MUTEX _T("Global\FileMapingMutex")

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
} SHARED_THREAD;

#endif UTILS_H
