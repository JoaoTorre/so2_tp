#include "utils.h"


int _tmain(int argc, TCHAR* argv[]) {

	HANDLE hSemaphoreArbitro;

#ifdef UNICODE 
	_setmode(_fileno(stdin), _O_WTEXT);
	_setmode(_fileno(stdout), _O_WTEXT);
	_setmode(_fileno(stderr), _O_WTEXT);
#endif
	
	hSemaphoreArbitro = CreateSemaphore(NULL, 1, 1, SEMAPHORE_UNIQUE_ARBITRO_NAME);

	if (hSemaphoreArbitro == NULL) {
		_ftprintf(stderr, _T("[ERRO] - Falha ao criar o semáforo: %d\n"), GetLastError());
		return -1;
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		_ftprintf(stderr, _T("[ERRO] - Já existe um Árbitro a decorrer"));
	}



	return 0;
}