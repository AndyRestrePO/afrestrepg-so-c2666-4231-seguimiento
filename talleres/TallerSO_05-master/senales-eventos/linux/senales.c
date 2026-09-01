#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

void esperar_senal(void);
void verifica_estado(int estado);
void manejador_senales(int signo);

int
main(int argc, char* const argv[], char* const env[]) {

  int estado;
  sigset_t conjunto;

  /*
   * La senal se bloquea en el padre ANTES del fork para que el
   * hijo la herede bloqueada desde su creacion. Asi, si el padre
   * envia la senal antes de que el hijo instale su manejador, esta
   * queda pendiente en vez de aplicarsele la disposicion por
   * omision (evita una condicion de carrera).
   */

  /* Hijo que captura SIGHUP */
  sigemptyset(&conjunto);
  sigaddset(&conjunto, SIGHUP);
  sigprocmask(SIG_BLOCK, &conjunto, NULL);

  pid_t pid_hijo_sighup = fork();

  if (pid_hijo_sighup == 0) {
    signal(SIGHUP, manejador_senales);
    sigprocmask(SIG_UNBLOCK, &conjunto, NULL);
    esperar_senal();
    _exit(EXIT_SUCCESS);
  }

  kill(pid_hijo_sighup, SIGHUP);
  sigprocmask(SIG_UNBLOCK, &conjunto, NULL);

  if (waitpid(pid_hijo_sighup, &estado, 0) == pid_hijo_sighup) {
    verifica_estado(estado);
  }

  /* Hijo que captura SIGQUIT */
  sigemptyset(&conjunto);
  sigaddset(&conjunto, SIGQUIT);
  sigprocmask(SIG_BLOCK, &conjunto, NULL);

  pid_t pid_hijo_sigquit = fork();

  if (pid_hijo_sigquit == 0) {
    signal(SIGQUIT, manejador_senales);
    sigprocmask(SIG_UNBLOCK, &conjunto, NULL);
    esperar_senal();
    _exit(EXIT_SUCCESS);
  }

  kill(pid_hijo_sigquit, SIGQUIT);
  sigprocmask(SIG_UNBLOCK, &conjunto, NULL);

  if (waitpid(pid_hijo_sigquit, &estado, 0) == pid_hijo_sigquit) {
    verifica_estado(estado);
  }

  return EXIT_SUCCESS;
}

void esperar_senal(void) {
  pause();
}

void manejador_senales(int signo) {
  switch (signo) {
  case SIGHUP:
    fprintf(stdout, "Senal capturada: SIGHUP (%d)\n", signo);
    break;
  case SIGQUIT:
    fprintf(stdout, "Senal capturada: SIGQUIT (%d)\n", signo);
    break;
  default:
    fprintf(stdout, "Senal capturada: %d\n", signo);
  }
  fflush(stdout);
  _exit(EXIT_SUCCESS);
}

void verifica_estado(int estado) {
  if (WIFEXITED(estado)) {
    fprintf(stdout, "Proceso termino invocando _exit: %d\n",
	    WEXITSTATUS(estado));
  }
  else if (WIFSIGNALED(estado)) {
    fprintf(stdout, "Proceso senalizado por: %d\n",
	    WTERMSIG(estado));
  }
  /* Se vacia el buffer para que el proximo fork() no arrastre
     lineas pendientes sin escribir hacia el proceso hijo. */
  fflush(stdout);
}
