OBJ = main.o game.o utils.o
FLAGS = -lncurses -std=c99
tetris: ${OBJ}
	${CC} ${FLAGS} -o $@ ${OBJ}  

clean: tetris 
	rm -f ${OBJ}
