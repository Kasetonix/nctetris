CSTD = gnu99
SRC = main.c game.c draw.c utils.c
OBJ = ${SRC:.c=.o}
LIBS = -lncurses
CFLAGS = -std=${CSTD}

.c.o:
	${CC} -c ${CFLAGS} $<

tetris: ${OBJ}
	${CC} ${OBJ} ${LIBS} -o $@

clean: tetris 
	rm -f ${OBJ}
