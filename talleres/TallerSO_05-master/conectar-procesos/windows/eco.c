/*
 * eco <nombre-fichero> <cadena-traduccion> <linea-modulo-a-eliminar>
 *
 * eco lee el fichero y le pasa cada linea a cat, cat envia su salida
 * a tr (con la transformacion indicada) y tr devuelve el resultado a
 * eco, que lo imprime, eliminando las lineas cuyo numero es multiplo
 * de <linea-modulo-a-eliminar>.
 *
 *   eco --tuberiaA--> cat --tuberiaB--> tr --tuberiaC--> eco
 *
 * Requiere que cat.exe y tr.exe (p.ej. de Git for Windows o MSYS2)
 * esten disponibles en el PATH del sistema.
 */
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMANO_BLOQUE 4096

typedef struct {
  const char *nombreFichero;
  HANDLE hEscritura;
} DatosAlimentador;

DWORD WINAPI
hiloAlimentador(LPVOID lpParametro) {

  DatosAlimentador *datos = (DatosAlimentador *) lpParametro;
  HANDLE hArchivo;
  char buffer[TAMANO_BLOQUE];
  DWORD dwLeidos, dwEscritos;

  hArchivo = CreateFile(datos->nombreFichero, GENERIC_READ, FILE_SHARE_READ,
			 NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

  if (hArchivo == INVALID_HANDLE_VALUE) {
    fprintf(stderr, "eco - Error abriendo %s: %ld\r\n",
	    datos->nombreFichero, GetLastError());
    CloseHandle(datos->hEscritura);
    return 1;
  }

  while (ReadFile(hArchivo, buffer, TAMANO_BLOQUE, &dwLeidos, NULL)
	 && dwLeidos > 0) {
    if (!WriteFile(datos->hEscritura, buffer, dwLeidos, &dwEscritos, NULL)) {
      break;
    }
  }

  CloseHandle(hArchivo);
  CloseHandle(datos->hEscritura); /* Senala EOF a cat */

  return 0;
}

static HANDLE
crearProceso(char *lineaComando, HANDLE hEntrada, HANDLE hSalida) {

  STARTUPINFO startupInfo;
  PROCESS_INFORMATION procInfo;

  ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
  startupInfo.cb = sizeof(STARTUPINFO);
  startupInfo.hStdInput  = hEntrada;
  startupInfo.hStdOutput = hSalida;
  startupInfo.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
  startupInfo.dwFlags   |= STARTF_USESTDHANDLES;

  ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));

  if (!CreateProcess(NULL, lineaComando, NULL, NULL, TRUE, 0, NULL, NULL,
		      &startupInfo, &procInfo)) {
    fprintf(stderr, "eco - Error creando proceso '%s': %ld\r\n",
	    lineaComando, GetLastError());
    ExitProcess((DWORD) EXIT_FAILURE);
  }

  CloseHandle(procInfo.hThread);

  return procInfo.hProcess;
}

int
main(int argc, char *argv[]) {

  if (argc != 4) {
    fprintf(stderr,
	    "Uso: %s <nombre-fichero> <cadena-traduccion> <linea-modulo-a-eliminar>\r\n",
	    argv[0]);
    ExitProcess((DWORD) EXIT_FAILURE);
  }

  int moduloEliminar = atoi(argv[3]);

  if (moduloEliminar <= 0) {
    fprintf(stderr, "El modulo a eliminar debe ser un entero mayor que 0\r\n");
    ExitProcess((DWORD) EXIT_FAILURE);
  }

  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = NULL;

  HANDLE tuberiaA_lectura, tuberiaA_escritura;
  HANDLE tuberiaB_lectura, tuberiaB_escritura;
  HANDLE tuberiaC_lectura, tuberiaC_escritura;

  /* Tuberia eco -> cat. La escritura solo la usa el hilo alimentador
     (dentro de este mismo proceso), nunca debe heredarse. */
  if (!CreatePipe(&tuberiaA_lectura, &tuberiaA_escritura, &saAttr, 0)) {
    fprintf(stderr, "Error creando tuberia A: %ld\r\n", GetLastError());
    ExitProcess((DWORD) EXIT_FAILURE);
  }
  SetHandleInformation(tuberiaA_escritura, HANDLE_FLAG_INHERIT, 0);

  DatosAlimentador datosAlimentador;
  datosAlimentador.nombreFichero = argv[1];
  datosAlimentador.hEscritura = tuberiaA_escritura;

  HANDLE hHiloAlimentador = CreateThread(NULL, 0, hiloAlimentador,
					  &datosAlimentador, 0, NULL);

  if (hHiloAlimentador == NULL) {
    fprintf(stderr, "Error creando el hilo alimentador: %ld\r\n",
	    GetLastError());
    ExitProcess((DWORD) EXIT_FAILURE);
  }

  /* Tuberia cat -> tr. Se crea antes de lanzar cat, pero el extremo
     de lectura se deshabilita temporalmente para que cat no lo
     herede; se vuelve a habilitar justo antes de crear tr. */
  if (!CreatePipe(&tuberiaB_lectura, &tuberiaB_escritura, &saAttr, 0)) {
    fprintf(stderr, "Error creando tuberia B: %ld\r\n", GetLastError());
    ExitProcess((DWORD) EXIT_FAILURE);
  }
  SetHandleInformation(tuberiaB_lectura, HANDLE_FLAG_INHERIT, 0);

  char comandoCat[] = "cat";
  HANDLE hProcesoCat = crearProceso(comandoCat, tuberiaA_lectura,
				     tuberiaB_escritura);

  CloseHandle(tuberiaA_lectura);
  CloseHandle(tuberiaB_escritura);

  /* Tuberia tr -> eco. El extremo de lectura lo usara el propio
     proceso eco, nunca debe heredarse. */
  if (!CreatePipe(&tuberiaC_lectura, &tuberiaC_escritura, &saAttr, 0)) {
    fprintf(stderr, "Error creando tuberia C: %ld\r\n", GetLastError());
    ExitProcess((DWORD) EXIT_FAILURE);
  }
  SetHandleInformation(tuberiaC_lectura, HANDLE_FLAG_INHERIT, 0);

  SetHandleInformation(tuberiaB_lectura, HANDLE_FLAG_INHERIT,
		       HANDLE_FLAG_INHERIT);

  char comandoTr[512];
  snprintf(comandoTr, sizeof(comandoTr), "tr %s", argv[2]);

  HANDLE hProcesoTr = crearProceso(comandoTr, tuberiaB_lectura,
				    tuberiaC_escritura);

  CloseHandle(tuberiaB_lectura);
  CloseHandle(tuberiaC_escritura);

  /* eco lee el resultado final desde tuberiaC_lectura, separando
     lineas e imprimiendo todas salvo las multiplos del modulo. */
  char bloque[TAMANO_BLOQUE];
  char *acumulado = NULL;
  size_t acumLen = 0, acumCap = 0;
  DWORD dwLeidos;
  int numeroLinea = 0;

  while (ReadFile(tuberiaC_lectura, bloque, TAMANO_BLOQUE, &dwLeidos, NULL)
	 && dwLeidos > 0) {

    if (acumLen + dwLeidos > acumCap) {
      acumCap = (acumLen + dwLeidos) * 2;
      acumulado = (char *) realloc(acumulado, acumCap);
    }
    memcpy(acumulado + acumLen, bloque, dwLeidos);
    acumLen += dwLeidos;

    size_t inicio = 0;
    size_t i;
    for (i = 0; i < acumLen; i++) {
      if (acumulado[i] == '\n') {
	size_t largoLinea = i - inicio + 1;
	numeroLinea++;
	if (numeroLinea % moduloEliminar != 0) {
	  fwrite(acumulado + inicio, 1, largoLinea, stdout);
	}
	inicio = i + 1;
      }
    }

    size_t restante = acumLen - inicio;
    memmove(acumulado, acumulado + inicio, restante);
    acumLen = restante;
  }

  if (acumLen > 0) {
    numeroLinea++;
    if (numeroLinea % moduloEliminar != 0) {
      fwrite(acumulado, 1, acumLen, stdout);
    }
  }

  free(acumulado);
  fflush(stdout);
  CloseHandle(tuberiaC_lectura);

  WaitForSingleObject(hHiloAlimentador, INFINITE);
  WaitForSingleObject(hProcesoCat, INFINITE);
  WaitForSingleObject(hProcesoTr, INFINITE);

  CloseHandle(hHiloAlimentador);
  CloseHandle(hProcesoCat);
  CloseHandle(hProcesoTr);

  ExitProcess((DWORD) EXIT_SUCCESS);
}
