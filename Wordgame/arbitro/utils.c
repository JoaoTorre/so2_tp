#include "utils.h"
#include "struct.h"


TCHAR* toUpperString(TCHAR* string) {
	for (unsigned int i = 0; i < _tcslen(string); i++) {
		string[i] = _toupper(string[i]);
	}

	return string;
}


void RemoveNovaLinha(TCHAR* str) {
	size_t len = _tcslen(str);
	if (len > 0 && str[len - 1] == '\n') {
		str[len - 1] = '\0';
	}
}

TCHAR** splitString(TCHAR* str, const TCHAR* delim, unsigned int* size) {
    TCHAR* nextToken = NULL, ** temp, ** returnArray = NULL;
    TCHAR* token = _tcstok_s(str, delim, &nextToken);

    // Verifica se a string é NULL ou vazia
    if (str == NULL || _tcslen(str) == 0) {
        _tprintf(_T("[ERRO] String vazia!\n"));
        *size = 0; // Define o tamanho como 0
        return NULL; // Retorna NULL para indicar que não há tokens
    }

    *size = 0;

    while (token != NULL) {
        temp = (TCHAR**)realloc(returnArray, sizeof(TCHAR*) * (*size + 1));
        if (temp == NULL) {
            _tprintf(_T("[ERRO] Impossível alocar memória para string!\n"));
            free(returnArray);
            *size = 0;
            return NULL;
        }

        returnArray = temp;
        returnArray[(*size)++] = token;

        token = _tcstok_s(NULL, delim, &nextToken);
    }

    return returnArray;
}


BOOL getValueFromKeyMAXLETRAS(unsigned int* maxLetras) {
	LSTATUS res;
	HKEY chave;
	DWORD tamanhoValor;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, KEY_PATH, 0, KEY_READ, &chave);
	if (res != ERROR_SUCCESS) {
		_ftprintf(stderr, _T("[ERRO] - Ao abrir a chave de registro: %d\n"), res);
		*maxLetras = 0;
		return FALSE;
	}

	res = RegQueryValueEx(chave, KEY_MAXLETRAS, NULL, NULL, (LPBYTE)maxLetras, &tamanhoValor);

	if (res != ERROR_SUCCESS) {
		_ftprintf(stderr, _T("[ERRO] - Ao aceder valor: %d\n"), res);
		*maxLetras = 0;
		RegCloseKey(chave);
		return FALSE;
	}

	RegCloseKey(chave); 
	return TRUE;
}

BOOL getValueFromKeyRITMO(unsigned int* nRitmo) {
	LSTATUS res;
	HKEY chave;
	DWORD tamanhoValor;

	res = RegOpenKeyEx(HKEY_CURRENT_USER, KEY_PATH, 0, KEY_READ, &chave);
	if (res != ERROR_SUCCESS) {
		_ftprintf(stderr, _T("[ERRO] - Ao abrir a chave de registro: %d\n"), res);
		*nRitmo = 0;
		return FALSE;
	}

	res = RegQueryValueEx(chave, KEY_RITMO, NULL, NULL, (LPBYTE)nRitmo, &tamanhoValor);
	if (res != ERROR_SUCCESS) {
		_ftprintf(stderr, _T("[ERRO] - Ao aceder valor: %d\n"), res);
		*nRitmo = 0;
		RegCloseKey(chave);
		return FALSE;
	}

	RegCloseKey(chave); 
	return TRUE;
}

BOOL setValueToKeyRITMO(unsigned int nRitmo){
	LSTATUS res;
	HKEY chave;

	res = RegCreateKeyEx(HKEY_CURRENT_USER, KEY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &chave, NULL);
	if (res != ERROR_SUCCESS) {
		_ftprintf(stderr, _T("[ERRO] - Ao abrir a chave de registro: %d\n"), res);
		return FALSE;
	}

	res = RegSetValueEx(chave, KEY_RITMO, 0, REG_DWORD, (const BYTE*)&nRitmo, sizeof(nRitmo));
	if (res != ERROR_SUCCESS) {
		_ftprintf(stderr, _T("[ERRO] - Ao guardar valor: %d\n"), res);
		RegCloseKey(chave);
		return FALSE;
	}

	RegCloseKey(chave);
	return TRUE;
}

BOOL setValueToKeyMAXLETRAS(unsigned int maxLetras){
	LSTATUS res;
	HKEY chave;
	
	res = RegCreateKeyEx(HKEY_CURRENT_USER, KEY_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &chave, NULL);
	if (res != ERROR_SUCCESS) {
		_ftprintf(stderr, _T("[ERRO] - Ao abrir a chave de registro: %d\n"), res);
		return FALSE;
	}
	
	res = RegSetValueEx(chave, KEY_MAXLETRAS, 0, REG_DWORD, (const BYTE*)&maxLetras, sizeof(maxLetras));
	if (res != ERROR_SUCCESS) {
		_ftprintf(stderr, _T("[ERRO] - Ao guardar valor: %d\n"), res);
		RegCloseKey(chave);
		return FALSE;
	}

	RegCloseKey(chave);
	return TRUE;
}
