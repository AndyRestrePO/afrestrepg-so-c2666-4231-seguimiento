#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

void abrirArchivo(const char *nombre) {
    int estado = open(nombre, O_RDONLY);

    if (estado < 0) {
        fprintf(stderr, "Error abriendo %s debido a %d\n", nombre, errno);
    } else {
        printf("Archivo %s abierto correctamente\n", nombre);
        close(estado);
    }

    errno = 0;
}

int main(void) {
    abrirArchivo("no-existe.txt");
    abrirArchivo("existe.txt");

    return 0;
}
