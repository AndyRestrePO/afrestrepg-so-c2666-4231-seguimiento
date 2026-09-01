#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

void esperar_senal(void);
void verifica_estado(int estado);
void manejador_sigchld(int signo);

int
main(int argc, char* const argv[], char* const env[]) {
  pid_t pid_hijo_no_maneja_senal = fork();

  if (pid_hijo_no_maneja_senal == 0) {
    esperar_senal();
    _exit(EXIT_FAILURE);
  }

  kill(pid_hijo_no_maneja_senal, SIGALRM);

  int estado;
  if (waitpid(pid_hijo_no_maneja_senal, &estado, 0) == pid_hijo_no_maneja_senal) {
    verifica_estado(estado);
  }

  /*
   * SIGCHLD se ignora por omision, por lo que el hijo original
   * jamas terminaba al recibirla. Se instala un manejador para
   * capturarla en lugar de dejar que se ignore. La senal se
   * bloquea en el padre antes del fork para que el hijo la herede
   * bloqueada desde su creacion: si el padre la envia antes de que
   * el hijo instale el manejador, queda pendiente en vez de
   * perderse por la disposicion por omision (evita una condicion
   * de carrera).
   */
  sigset_t conjunto;
  sigemptyset(&conjunto);
  sigaddset(&conjunto, SIGCHLD);
  sigprocmask(SIG_BLOCK, &conjunto, NULL);

  pid_t pid_hijo_ignora_senal = fork();

  if (pid_hijo_ignora_senal == 0) {
    signal(SIGCHLD, manejador_sigchld);
    sigprocmask(SIG_UNBLOCK, &conjunto, NULL);
    esperar_senal();
    _exit(EXIT_FAILURE);
  }

  kill(pid_hijo_ignora_senal, SIGCHLD);
  sigprocmask(SIG_UNBLOCK, &conjunto, NULL);

  int tiempo_espera = 5;
  alarm(tiempo_espera);

  pid_t hijo_esperado = waitpid(pid_hijo_ignora_senal, &estado, 0);
  if (hijo_esperado == pid_hijo_ignora_senal) {
    verifica_estado(estado);
  }

  return EXIT_SUCCESS;
}

void esperar_senal(void) {
  pause();
}

void manejador_sigchld(int signo) {
  fprintf(stdout, "Senal capturada (ya no se ignora): %d\n", signo);
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
