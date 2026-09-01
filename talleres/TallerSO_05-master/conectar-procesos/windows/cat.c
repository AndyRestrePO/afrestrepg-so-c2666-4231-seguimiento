/*
 * cat.c
 *
 * Version minima de "cat": copia la entrada estandar en la salida
 * estandar. Se usa como etapa intermedia de la tuberia de eco.c, ya
 * que un sistema Windows recien instalado no trae un "cat" real.
 */
#include <windows.h>
#include <stdio.h>

#define BUFFER_SIZE 4096

int
main(int argc, char *argv[]) {

  HANDLE hStdInput, hStdOutput;
  char buffer[BUFFER_SIZE];
  DWORD dwBytesRead, dwBytesWritten;

  hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
  hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

  while (ReadFile(hStdInput, (PVOID) buffer, (DWORD) BUFFER_SIZE,
		   &dwBytesRead, NULL)) {

    if (dwBytesRead == 0) {
      break;
    }

    if (!WriteFile(hStdOutput, (PVOID) buffer, dwBytesRead,
		   &dwBytesWritten, NULL)) {
      fprintf(stderr, "cat - Error escribiendo: %ld\r\n", GetLastError());
      ExitProcess((DWORD) EXIT_FAILURE);
    }
  }

  ExitProcess((DWORD) EXIT_SUCCESS);
  return 0;
}
