#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int
main() {

  pid_t hijo1, hijo2;
  int status;
  int pipe1[2];

  pipe(pipe1);

  hijo1 = fork();

  if (hijo1 == 0) { /* Primer hijo: escribe en la tuberia */
    dup2(pipe1[1], STDOUT_FILENO);
    close(pipe1[0]);
    close(pipe1[1]);
    execl("/bin/ls", "ls", "-l", NULL);
    _exit(1);
  }

  hijo2 = fork();

  if (hijo2 == 0) { /* Segundo hijo: lee de la tuberia */
    dup2(pipe1[0], STDIN_FILENO);
    close(pipe1[0]);
    close(pipe1[1]);
    execl("/usr/bin/wc", "wc", NULL);
    _exit(1);
  }

  /* El padre no necesita ninguno de los extremos de la tuberia */
  close(pipe1[0]);
  close(pipe1[1]);

  waitpid(hijo1, &status, 0);
  waitpid(hijo2, &status, 0);

  return 0;
}
