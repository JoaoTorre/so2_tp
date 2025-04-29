#include "utils.h"
#include "struct.h"

TCHAR** splitString(TCHAR* str, const TCHAR* delim, unsigned int* size) {
    TCHAR* nextToken = NULL, ** temp, ** returnArray = NULL;
    TCHAR* token = _tcstok_s(str, delim, &nextToken);

    // Verifica se a string � NULL ou vazia
    if (str == NULL || _tcslen(str) == 0) {
        _tprintf(_T("[ERRO] String vazia!\n"));
        *size = 0; // Define o tamanho como 0
        return NULL; // Retorna NULL para indicar que n�o h� tokens
    }

    *size = 0;

    while (token != NULL) {
        temp = (TCHAR**)realloc(returnArray, sizeof(TCHAR*) * (*size + 1));
        if (temp == NULL) {
            _tprintf(_T("[ERRO] Imposs�vel alocar mem�ria para string!\n"));
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