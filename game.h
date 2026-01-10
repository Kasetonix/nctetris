#pragma once

#include <stdbool.h>
#include "utils.h"

#define FRAMERATE 30
#define FRAMETIME ((f64) (1.0 / FRAMERATE))
#define LOCKDOWN_FRAMES (FRAMERATE / 2)
#define BLINK_FRAMES 5
#define ENTRY_DELAY 3
#define FLOOR_MOVES 15

#define TM_SIZE 4
#define TM_NUM 7
#define TM_ORIENT 4
#define TM_ROT_DIRS 2

#define WK_TESTS 4

#define BAG_SIZE 7

#define FIELD_UM 2
#define FIELD_X 10
#define FIELD_Y (20 + FIELD_UM)
#define BORDER_THICKNESS 1

#define BLOCK_X 4
#define BLOCK_Y 2

#define GRAVITY_ARR_SIZE 20

#define DRAW_CHAR ' '
#define GHOST_CHAR '*'

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
    u8 level;
    i8 combo;
    u8 bag[BAG_SIZE];
    u8 bag_index;
    Tetromino tm_field;
    Tetromino tm_next;
    Tetromino tm_hold;
    bool swapped;
    u8 field[FIELD_Y][FIELD_X];
    u8 gravity_timer;
    u8 floor_timer;
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

Tetromino tm_create_rand(Game *game);
bool tm_fits(Game *game, Tetromino *tm, Vec offset);
bool tm_on_floor(Game *game, Tetromino *tm);
bool tm_spawn(Game *game);
bool tick(Game *game, i16 ch);
