#include <curses.h>
#include <stdint.h>
#include <time.h>

#define WINDOW_NUM 5
#define MAX_TITLE_LEN 32

#define EMPTY_CHAR ' '

#define WINT_FIELD ""
#define WINT_HOLDTM "HOLD"
#define WINT_NEXTTM "NEXT"
#define WINT_SCORE "SCORE"
#define WINT_LEVEL "LEVEL"

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
    WIN_FIELD, WIN_HOLDTM, WIN_NEXTTM, WIN_SCORE, WIN_LEVEL
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
f32 time_since(clock_t clock);
u64 us(f32 time);
