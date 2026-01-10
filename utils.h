#pragma once

#include <curses.h>
#include <stdint.h>
#include <time.h>

// basic variables
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef float f32;
typedef double f64;

typedef struct Windim {
    u16 rows;
    u16 cols;
} Windim;

typedef enum {
    WIN_FIELD, WIN_HOLDTM, WIN_NEXTTM, WIN_SCORE, WIN_LEVEL, WIN_DEBUG
} WindowID;

typedef enum {
    WHITE, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, BLACK
} Color;

// ncurses
void init_ncurses();
WINDOW *create_win(u16 y, u16 x, u16 height, u16 width);
void border_draw(WINDOW *win, char *title);
Windim get_scrdim();

// general
struct timespec ns_to_timespec(f64 time);
struct timespec time_to_sleep(struct timespec time);
