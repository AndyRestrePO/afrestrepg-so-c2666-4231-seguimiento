/*
 * tr.c <conjunto-origen> <conjunto-destino>
 *
 * Version minima de "tr": traduce, byte a byte, los caracteres de la
 * entrada estandar que aparecen en <conjunto-origen> por el caracter
 * correspondiente de <conjunto-destino>, y copia el resto sin
 * cambios. Admite rangos tipo "a-z". Se usa como etapa intermedia de
 * la tuberia de eco.c, ya que un sistema Windows recien instalado no
 * trae un "tr" real.
 */
#include <windows.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 4096

static int
expandirConjunto(const char *entrada, char *salida, int capacidadSalida) {

  int i = 0, j = 0;
  int n = (int) strlen(entrada);

  while (i < n && j < capacidadSalida) {
    if (i + 2 < n && entrada[i + 1] == '-') {
      unsigned char desde = (unsigned char) entrada[i];
      unsigned char hasta = (unsigned char) entrada[i + 2];
      unsigned char c;
      for (c = desde; c <= hasta && j < capacidadSalida; c++) {
	salida[j++] = (char) c;
      }
      i += 3;
    }
    else {
      salida[j++] = entrada[i++];
    }
  }

  return j;
}

int
main(int argc, char *argv[]) {

  unsigned char tabla[256];
  int i;

  for (i = 0; i < 256; i++) {
    tabla[i] = (unsigned char) i;
  }

  if (argc >= 3) {
    char conjunto1[256], conjunto2[256];
    int len1 = expandirConjunto(argv[1], conjunto1, sizeof(conjunto1));
    int len2 = expandirConjunto(argv[2], conjunto2, sizeof(conjunto2));

    if (len1 > 0 && len2 > 0) {
      for (i = 0; i < len1; i++) {
	int posDestino = (i < len2) ? i : (len2 - 1);
	tabla[(unsigned char) conjunto1[i]] = (unsigned char) conjunto2[posDestino];
      }
    }
  }

  HANDLE hStdInput, hStdOutput;
  char buffer[BUFFER_SIZE];
  DWORD dwBytesRead, dwBytesWritten, k;

  hStdInput  = GetStdHandle(STD_INPUT_HANDLE);
  hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);

  while (ReadFile(hStdInput, (PVOID) buffer, (DWORD) BUFFER_SIZE,
		   &dwBytesRead, NULL)) {

    if (dwBytesRead == 0) {
      break;
    }

    for (k = 0; k < dwBytesRead; k++) {
      buffer[k] = (char) tabla[(unsigned char) buffer[k]];
    }

    if (!WriteFile(hStdOutput, (PVOID) buffer, dwBytesRead,
		   &dwBytesWritten, NULL)) {
      fprintf(stderr, "tr - Error escribiendo: %ld\r\n", GetLastError());
      ExitProcess((DWORD) EXIT_FAILURE);
    }
  }

  ExitProcess((DWORD) EXIT_SUCCESS);
  return 0;
}
