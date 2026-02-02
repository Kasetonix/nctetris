CSTD = gnu99
SRC = utils.c game.c draw.c main.c
OBJ = ${SRC:.c=.o}
LIBS = -lcurses
CFLAGS = -std=${CSTD}

tetris: ${OBJ}
	${CC} ${OBJ} ${LIBS} -o $@

clean: tetris 
	rm -f ${OBJ}
