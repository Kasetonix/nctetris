#include <curses.h>
#include <locale.h>
#include <time.h>
#include "utils.h"

// ncurses

// initializes the ncurses library 
void init_ncurses() {
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    timeout(0);
    curs_set(0);

    start_color();
    for (u8 i = 0; i < 8; i++)
        init_pair(i, i, COLOR_BLACK);
}

// creates a new window
WINDOW *create_win(u16 y, u16 x, u16 height, u16 width) {
    WINDOW *win;
    win = newwin(height, width, y, x);
    wrefresh(win);
    return win;
}

void border_draw(WINDOW *win, char *title) {
    box(win, 0, 0);
    if (title[0] != '\0') // draw title if not empty
        mvwprintw(win, 0, 1, "|%s|", title);
    wrefresh(win);
}

// gets the dimensions of stdscr
inline Windim get_scrdim() {
    return (Windim) { (u16) getmaxy(stdscr), (u16) getmaxx(stdscr) };
}

// general

// calculates the time elapsed
inline f32 time_since(clock_t time) {
    return (clock() - time * 1.0) / CLOCKS_PER_SEC;
}

// Converts seconds to microseconds
inline u64 us(f32 time) {
    return (u64) time * 1000000L;
}
