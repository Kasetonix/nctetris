CSTD = gnu99
SRC = utils.c game.c draw.c main.c
OBJ = ${SRC:.c=.o}
LIBS = -lcurses
CFLAGS = -std=${CSTD}

.c.o:
	${CC} -c ${CFLAGS} $<

tetris: ${OBJ}
	${CC} ${OBJ} ${LIBS} -o $@

debug:
	${CC} -c ${CFLAGS} -Wall -Wextra -Werror ${SRC}

clean: tetris 
	rm -f ${OBJ}
