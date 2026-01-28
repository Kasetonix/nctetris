#include <curses.h>
#include <locale.h>
#include <time.h>
#include "utils.h"
#include "game.h"

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
    return win;
}

// draws a border around a window
void border_draw(WINDOW *win, char *title) {
    box(win, 0, 0);
    if (title[0] != '\0') // draw title if not empty
        mvwprintw(win, 0, 1, "|%s|", title);
    wnoutrefresh(win);
}

// gets the dimensions of stdscr
inline Windim get_scrdim() {
    return (Windim) { (u16) getmaxy(stdscr), (u16) getmaxx(stdscr) };
}

// translates a float containing seconds into a timespec representation
struct timespec ns_to_timespec(f64 time) {
    struct timespec time_ts;
    time_ts.tv_sec = (u64) time;
    time_ts.tv_nsec = (__time_t) ((time - time_ts.tv_sec) * 1.0e9);
    return time_ts;
}

// Calculates the time needed to sleep to achieve constant framerate
struct timespec time_to_sleep(struct timespec time) {
    struct timespec time_now, sleep_ts, frametime;
    clock_gettime(CLOCK_REALTIME, &time_now);
    frametime = ns_to_timespec(FRAMETIME);
    sleep_ts.tv_sec  = frametime.tv_sec  - (time_now.tv_sec  - time.tv_sec);
    sleep_ts.tv_nsec = frametime.tv_nsec - (time_now.tv_nsec - time.tv_nsec);
    if (sleep_ts.tv_nsec <= 0) {
        sleep_ts.tv_sec--;
        sleep_ts.tv_nsec += 1000000000;
    }

    return sleep_ts;
}
