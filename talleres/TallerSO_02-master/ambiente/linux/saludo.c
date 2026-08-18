#include <stdio.h>
#include <unistd.h>

void uso(const char *progname, int valor_retorno);

int main(int argc, char *argv[]) {
    extern int optind;
    int option;
    int despedida = 0;

    while ((option = getopt(argc, argv, "hsd")) != -1) {
        switch (option) {
            case 'h':
                uso(argv[0], 0);
                break;

            case 's':
                despedida = 0;
                break;

            case 'd':
                despedida = 1;
                break;

            default:
                uso(argv[0], 1);
                break;
        }
    }

    if (optind >= argc) {
        uso(argv[0], 1);
    }

    if (despedida)
        printf("Adios %s\n", argv[optind]);
    else
        printf("Hola %s\n", argv[optind]);

    return 0;
}

void uso(const char *nombre_programa, int valor_retorno) {
    printf("Uso: %s -h\n", nombre_programa);
    printf("     %s [-s|-d] <nombre>\n", nombre_programa);
    _exit(valor_retorno);
}
