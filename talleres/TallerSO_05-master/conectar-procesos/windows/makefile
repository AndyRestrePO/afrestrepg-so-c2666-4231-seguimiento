.phony: clean

SRCS=conectar-procesos.c concatenar.c mostrar-archivo.c eco.c cat.c tr.c
OBJS=$(SRCS:.c=.o)
EXECS=$(OBJS:.o=)

all: ${EXECS}

conectar-procesos: conectar-procesos.o

conectar-procesos.o: conectar-procesos.c

concatenar: concatenar.o

concatenar.o: concatenar.c

mostrar-archivo: mostrar-archivo.o

mostrar-archivo.o: mostrar-archivo.c

eco: eco.o

eco.o: eco.c

cat: cat.o

cat.o: cat.c

tr: tr.o

tr.o: tr.c

clean:
	rm -f ${OBJS}
	rm -f ${EXECS}
	rm -f *~
