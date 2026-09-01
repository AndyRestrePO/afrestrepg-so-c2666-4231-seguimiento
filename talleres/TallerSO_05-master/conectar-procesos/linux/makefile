.phony: clean

SRCS=conectar-procesos.c conectar-procesos2.c eco.c
OBJS=$(SRCS:.c=.o)
EXECS=conectar-procesos conectar-procesos2 eco

all: ${EXECS}

conectar-procesos: conectar-procesos.o
	${CC} -o $@ $<

conectar-procesos2: conectar-procesos2.o
	${CC} -o $@ $<

eco: eco.o
	${CC} -o $@ $<

clean:
	rm -f ${OBJS}
	rm -f ${EXECS}
	rm -f *~
