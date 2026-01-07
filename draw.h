#pragma once

#include "game.h"

void tm_draw(WINDOW *win, Vec block_size, Tetromino *tm, bool ghost);
void tm_nh_draw(WINDOW *win, Vec block_size, Tetromino *tm);
void tm_draw_ghost(WINDOW *win, Vec block_size, Game *game, Tetromino *tm);
void field_draw(WINDOW *w_field, Vec block_size, Game *game);
void print_score(WINDOW *w_score, u32 score);
void print_level(WINDOW *w_level, u8 level);
