#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

/*
 * eco <nombre-fichero> <cadena-traduccion> <linea-modulo-a-eliminar>
 *
 * eco lee el fichero y le pasa cada linea a cat, cat envia su salida
 * a tr (con la transformacion indicada) y tr devuelve el resultado a
 * eco, que lo imprime, eliminando las lineas cuyo numero es multiplo
 * de <linea-modulo-a-eliminar>.
 *
 *   eco --pipeA--> cat --pipeB--> tr --pipeC--> eco
 */

static void cerrar_pipe(int p[2]);
static char **tokenizar(char *cadena, int *ntokens);

int
main(int argc, char *argv[]) {

  if (argc != 4) {
    fprintf(stderr,
	    "Uso: %s <nombre-fichero> <cadena-traduccion> <linea-modulo-a-eliminar>\n",
	    argv[0]);
    exit(EXIT_FAILURE);
  }

  const char *nombreFichero = argv[1];
  int moduloEliminar = atoi(argv[3]);

  if (moduloEliminar <= 0) {
    fprintf(stderr, "El modulo a eliminar debe ser un entero mayor que 0\n");
    exit(EXIT_FAILURE);
  }

  int ntokens;
  char **argvTr = tokenizar(argv[2], &ntokens);

  int pipeA[2]; /* eco   -> cat */
  int pipeB[2]; /* cat   -> tr  */
  int pipeC[2]; /* tr    -> eco */

  if (pipe(pipeA) == -1 || pipe(pipeB) == -1 || pipe(pipeC) == -1) {
    perror("pipe");
    exit(EXIT_FAILURE);
  }

  /* Hijo que alimenta la tuberia con el contenido del fichero */
  pid_t pidAlimentador = fork();

  if (pidAlimentador == 0) {
    close(pipeA[0]);
    cerrar_pipe(pipeB);
    cerrar_pipe(pipeC);

    FILE *fichero = fopen(nombreFichero, "r");
    if (fichero == NULL) {
      perror("fopen");
      _exit(EXIT_FAILURE);
    }

    char *linea = NULL;
    size_t capacidad = 0;
    ssize_t leidos;

    while ((leidos = getline(&linea, &capacidad, fichero)) != -1) {
      write(pipeA[1], linea, leidos);
    }

    free(linea);
    fclose(fichero);
    close(pipeA[1]);
    _exit(EXIT_SUCCESS);
  }

  /* Hijo que ejecuta cat, leyendo de pipeA y escribiendo en pipeB */
  pid_t pidCat = fork();

  if (pidCat == 0) {
    dup2(pipeA[0], STDIN_FILENO);
    dup2(pipeB[1], STDOUT_FILENO);
    cerrar_pipe(pipeA);
    cerrar_pipe(pipeB);
    cerrar_pipe(pipeC);
    execlp("cat", "cat", NULL);
    perror("execlp cat");
    _exit(EXIT_FAILURE);
  }

  /* Hijo que ejecuta tr, leyendo de pipeB y escribiendo en pipeC */
  pid_t pidTr = fork();

  if (pidTr == 0) {
    dup2(pipeB[0], STDIN_FILENO);
    dup2(pipeC[1], STDOUT_FILENO);
    cerrar_pipe(pipeA);
    cerrar_pipe(pipeB);
    cerrar_pipe(pipeC);
    execvp("tr", argvTr);
    perror("execvp tr");
    _exit(EXIT_FAILURE);
  }

  /* El padre (eco) solo necesita el extremo de lectura de pipeC */
  cerrar_pipe(pipeA);
  cerrar_pipe(pipeB);
  close(pipeC[1]);

  FILE *entrada = fdopen(pipeC[0], "r");
  char *linea = NULL;
  size_t capacidad = 0;
  ssize_t leidos;
  int numeroLinea = 0;

  while ((leidos = getline(&linea, &capacidad, entrada)) != -1) {
    numeroLinea++;
    if (numeroLinea % moduloEliminar != 0) {
      fwrite(linea, 1, leidos, stdout);
    }
  }

  free(linea);
  fclose(entrada);

  waitpid(pidAlimentador, NULL, 0);
  waitpid(pidCat, NULL, 0);
  waitpid(pidTr, NULL, 0);

  free(argvTr);

  return EXIT_SUCCESS;
}

static void cerrar_pipe(int p[2]) {
  close(p[0]);
  close(p[1]);
}

static char **tokenizar(char *cadena, int *ntokens) {
  int capacidad = 4;
  char **tokens = malloc(sizeof(char *) * capacidad);
  int n = 0;

  char *token = strtok(cadena, " \t");
  while (token != NULL) {
    if (n + 1 >= capacidad) {
      capacidad *= 2;
      tokens = realloc(tokens, sizeof(char *) * capacidad);
    }
    tokens[n++] = token;
    token = strtok(NULL, " \t");
  }

  char **argvTr = malloc(sizeof(char *) * (n + 2));
  argvTr[0] = "tr";
  for (int i = 0; i < n; i++) {
    argvTr[i + 1] = tokens[i];
  }
  argvTr[n + 1] = NULL;

  free(tokens);
  *ntokens = n;
  return argvTr;
}
