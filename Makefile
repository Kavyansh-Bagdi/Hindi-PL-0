CC = cc
CFLAGS = -g -O2 -DHAVE_STRTONUM

PROG = hindipl0c
OBJS = hindipl0c.o strtonum.o

TEST_SCRIPT = tests/test.sh
TEST_MODE ?= -c   # default mode is -c (only generate .c)

all: ${OBJS}
	${CC} ${LDFLAGS} -o ${PROG} ${OBJS}

test:
	cd tests && bash ./test.sh ${TEST_MODE}

clean:
	rm -f ${PROG} ${OBJS} ${PROG}.core output/*.c output/*
