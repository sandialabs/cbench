CFLAGS+= -DMPI -DPARANOID -W -Wall
#CFLAGS= -DMPI -DPARANOID
LIBS= -lmpi
LIBS=

CC=/usr/local/cplant/ross/current/bin/cc
CC=mpicc

PGM= piob

OBJ= main.o client.o serv.o perform.o error.o prctim.o xprt.o
SRC= main.c client.c serv.c perform.c error.c prctim.c xprt.c

all: ${PGM}

${PGM}: $(OBJ)
	${CC} $(CFLAGS) -o ${PGM} $(OBJ) $(LIBS)

clean:
	${RM} ${PGM} $(OBJ) core errs
