.phony: clean

SRCS=manejar-senal.c captura-senal.c manejadores.c senales.c manejar-senal2.c
OBJS=$(SRCS:.c=.o)
EXECS=manejar-senal captura-senal senales manejar-senal2

all: ${EXECS}

manejar-senal: manejar-senal.o
	${CC} -o $@ $<

captura-senal: captura-senal.o manejadores.o
	${CC} -o $@ $^

captura-senal.o: captura-senal.c manejadores.h

manejadores.o: manejadores.c manejadores.h

senales: senales.o
	${CC} -o $@ $<

manejar-senal2: manejar-senal2.o
	${CC} -o $@ $<

clean:
	rm -f ${OBJS}
	rm -f ${EXECS}
	rm -f *~
