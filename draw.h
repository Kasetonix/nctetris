#pragma once

#include "game.h"

#define WINDOW_NUM 5
#define MAX_TITLE_LEN 32
#define PAUSE_CHAR '/'

#define WINT_FIELD ""
#define WINT_FIELD_PAUSED "PAUSED"
#define WINT_HOLDTM "HOLD"
#define WINT_NEXTTM "NEXT"
#define WINT_LEVEL "LEVEL"
#define WINT_SCORE "SCORE"

#define SECONDS_AFTER_PAUSE 1 

void tm_draw(WINDOW *win, Vec block_size, Tetromino *tm, bool ghost);
void tm_nh_draw(WINDOW *win, Vec block_size, Tetromino *tm);
void tm_draw_ghost(WINDOW *win, Vec block_size, Game *game, Tetromino *tm);
void field_draw(WINDOW *w_field, Vec block_size, Game *game);
void print_score(WINDOW *w_score, u32 score);
void print_level(WINDOW *w_level, u8 level);
void print_pause(WINDOW *win, Game *game);
bool pause_game(WINDOW *w_field, Game *game, i16 *ch);
