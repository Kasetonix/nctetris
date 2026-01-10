SRC = main.c game.c draw.c utils.c
OBJ = ${SRC:.c=.o}
LIBS = -lncurses
FLAGS = -std=gnu99

.c.o:
	${CC} -c ${FLAGS} $< ${LIBS}

tetris: ${OBJ}
	${CC} ${FLAGS} ${OBJ} ${LIBS} -o $@

clean: tetris 
	rm -f ${OBJ}
