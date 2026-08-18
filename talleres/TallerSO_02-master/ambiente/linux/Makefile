CC = gcc
CFLAGS = -Wall

all: saludo ambiente variable

saludo: saludo.c
	$(CC) $(CFLAGS) -o saludo saludo.c

ambiente: ambiente.c
	$(CC) $(CFLAGS) -o ambiente ambiente.c

variable: variable.c
	$(CC) $(CFLAGS) -o variable variable.c

clean:
	rm -f saludo ambiente variable
