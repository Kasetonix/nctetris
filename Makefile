CSTD = gnu99
SRC = utils.c game.c draw.c main.c
OBJ = ${SRC:.c=.o}
LIBS = -lcurses
CFLAGS = -std=${CSTD}

all: tetris

debug: CFLAGS += -Wall -Wextra -Werror -g
debug: tetris

release: CFLAGS += -O3
release: tetris

tetris: ${OBJ}
	${CC} ${OBJ} ${LIBS} -o $@

clean: tetris 
	rm -f ${OBJ}
