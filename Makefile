# pl0c Makefile

CC =		cc
CFLAGS =	-g -O2 -DHAVE_STRTONUM

PROG =	hindipl0c
OBJS =	hindipl0c.o strtonum.o

all: ${OBJS}
	${CC} ${LDFLAGS} -o ${PROG} ${OBJS}

test:
	cd tests && bash ./test.sh

clean:
	rm -f ${PROG} ${OBJS} ${PROG}.core