OBJ = main.o game.o draw.o utils.o
LIBS = -lncurses
FLAGS = -std=c99 -Wall -Werror
tetris: ${OBJ}
	${CC} ${FLAGS} -o $@ ${OBJ} ${LIBS}

clean: tetris 
	rm -f ${OBJ}
