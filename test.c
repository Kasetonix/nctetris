#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include "utils.c"

int main() {
    u16 ch;
    init_ncurses();
    ch = getch();
    endwin();
    if (ch == UINT16_MAX)
        printf(":3\n");

    return 0;
}
