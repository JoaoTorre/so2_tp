#include "utils.h"
#include "struct.h"


TCHAR* toUpperString(TCHAR* string) {
	for (unsigned int i = 0; i < _tcslen(string); i++) {
		string[i] = _toupper(string[i]);
	}

	return string;
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


// verifica dados do login
int verificaLogin(Jogador jogador) {

	return 0;
}


BOOL getValueFromKeyNLETRAS(unsigned int* nLetras) {
	LSTATUS res;
	HKEY chave;
	DWORD tamanhoValor;

	// Abrir a chave
	res = RegOpenKeyEx(HKEY_CURRENT_USER, KEY_PATH, 0, KEY_READ, &chave);
	if (res != ERROR_SUCCESS) {
		_tprintf_s(_T("Erro ao abrir a chave de registro: %d\n"), res);
		*nLetras = 0;
		return FALSE;
	}

	// Obter o valor
	res = RegQueryValueEx(chave, KEY_MAXLETRAS, NULL, NULL, (LPBYTE)nLetras, &tamanhoValor);

	if (res != ERROR_SUCCESS) {
		_tprintf_s(_T("Erro ao abrir a chave de registro: %d\n"), res);
		*nLetras = 0;
		return FALSE;
	}

	RegCloseKey(chave); // fechar a chave
	return TRUE;
}