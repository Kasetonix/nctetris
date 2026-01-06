#include <stdbool.h>
#include "utils.h"

#define FRAMERATE 30
#define FRAMETIME (1.0 / FRAMERATE)
#define FRAMETIME_US (FRAMETIME * 1e6)

#define TM_SIZE 4
#define TM_NUM 7
#define TM_ORIENT 4

#define WK_TESTS 4

#define FIELD_UM 1
#define FIELD_X 10
#define FIELD_Y 21
#define BORDER_THICKNESS 1

#define BLOCK_X 4
#define BLOCK_Y 2

#define LVL_NUM 20

#define DRAW_CHAR ' '
#define PREV_CHAR '*'

#define FLOOR_MOVES 15
#define LOCKDOWN_FRAMES (FRAMERATE / 2)
#define BLINK_FRAMES 5
#define ENTRY_DELAY 3
#define LINES_PER_LEVEL 10

#define CH_MV_LEFT KEY_LEFT
#define CH_MV_RIGHT KEY_RIGHT
#define CH_ROTATE_CW KEY_UP
#define CH_ROTATE_CCW 'z'
#define CH_SOFT_DROP KEY_DOWN 
#define CH_HARD_DROP ' '
#define CH_HOLD 'c'
#define CH_QUIT 'q'
#define CH_PAUSE 'p'

typedef struct Vec {
    i16 y;
    i16 x;
} Vec;

typedef struct BoundingBox {
    u8 top;
    u8 bottom;
    u8 left;
    u8 right;
} BoundingBox;

typedef struct Tetromino {
    u8 type;
    u8 orientation;
    Vec pos;
    Vec pos_nh;
    Vec block[TM_SIZE];
    BoundingBox bbox;
} Tetromino;

typedef struct Game {
    u32 score;
    u32 lines_cleared;
    Tetromino tm_field;
    Tetromino tm_next;
    bool swapped;
    Tetromino tm_hold;
    u8 field[FIELD_Y][FIELD_X];
    u8 gravity_timer;
    u8 floor_counter;
    u8 entry_delay;
    bool gravity_acted;
    bool paused;
    Vec block_size;
} Game;

typedef enum Tm_Type {
    TM_O, TM_Z, TM_S, TM_L, TM_J, TM_T, TM_I
} Tm_Type;

typedef enum Direction {
    LEFT, RIGHT, UP, DOWN
} Direction;


Tetromino tm_create_rand(Vec block_size);
bool tm_on_floor(Game *game, Tetromino *tm);
void tm_draw(WINDOW *win, Vec block_size, Tetromino *tm, bool preview);
void tm_nh_draw(WINDOW *win, Vec block_size, Tetromino *tm);
void tm_draw_preview(WINDOW *win, Vec block_size, Game *game, Tetromino *tm);
void field_draw(WINDOW *w_field, Vec block_size, Game *game);
bool tm_spawn(Game *game);
void print_score(WINDOW *w_score, u32 score);
void print_level(WINDOW *w_level, u8 level);
bool tick(Game *game, u16 ch);
