#include <curses.h>
#include <stdint.h>
#include <unistd.h>

#include "draw.h"
#include "utils.h"
#include "game.h"

// Draws a singular block
static void block_draw(WINDOW *win, Vec block_size, Vec pos, u8 color, bool ghost) {
    if (color == BLACK)
        return;
    if (pos.x < 0 || pos.y < 0)
        return;

    if (!ghost) {
        for (u8 y = 0; y < block_size.y; y++) {
            wmove(win, pos.y + y, pos.x);
            for (u8 x = 0; x < block_size.x; x++) {
                waddch(win, DRAW_CHAR | COLOR_PAIR(color));
            }
        }
    } else {
        for (u8 y = 0; y < block_size.y; y++) {
            wmove(win, pos.y + y, pos.x);
            for (u8 x = 0; x < block_size.x; x++) {
                waddch(win, GHOST_CHAR | COLOR_PAIR(color));
            }
        }
        
    }

    wnoutrefresh(win);
}

// Draws a tetromino
void tm_draw(WINDOW *win, Vec block_size, Tetromino *tm, bool ghost) {
    if (tm->type == BLACK)
        return;

    Vec drawing_pos;
    for (u8 i = 0; i < TM_SIZE; i++) {
        drawing_pos.y = block_size.y * (tm->pos.y + tm->block[i].y - FIELD_UM) + BORDER_THICKNESS;
        drawing_pos.x = block_size.x * (tm->pos.x + tm->block[i].x) + BORDER_THICKNESS;
        block_draw(win, block_size, drawing_pos, tm->type, ghost);
    }
}

// Draws a tetromino onto Next and Hold windows
void tm_nh_draw(WINDOW *win, Vec block_size, Tetromino *tm) {
    Vec drawing_pos;

    for (u8 i = 0; i < TM_SIZE; i++) {
        drawing_pos = tm->pos_nh;
        drawing_pos.y += block_size.y * tm->block[i].y;
        drawing_pos.x += block_size.x * tm->block[i].x;
        block_draw(win, block_size, drawing_pos, tm->type, false);
    }
}

// Draws a ghost tetromino; it's final position when hard dropped
void tm_draw_ghost(WINDOW *win, Vec block_size, Game *game, Tetromino *tm) {
    Tetromino tm_ghost = *tm;
    while (tm_fits(game, &tm_ghost, (Vec) { 1, 0 }))
        tm_ghost.pos.y++;
    tm_draw(win, block_size, &tm_ghost, true);
}

// Draws the entire game field
void field_draw(WINDOW *w_field, Vec block_size, Game *game) {
    Vec pos = { BORDER_THICKNESS, BORDER_THICKNESS };
    bool prev = false;

    for (u8 y = FIELD_UM; y < FIELD_Y; y++) {
        for (u8 x = 0; x < FIELD_X; x++) {
            block_draw(w_field, block_size, pos, game->field[y][x], prev);
            pos.x += block_size.x;
        }
        pos.x = BORDER_THICKNESS;
        pos.y += block_size.y;
        prev = false;
    } 
}

// Prints the score to the given window
void print_score(WINDOW *w_score, u32 score) {
    mvwprintw(w_score, BORDER_THICKNESS, BORDER_THICKNESS, "%i", score);
    wnoutrefresh(w_score);
}

// Prints the level to the given window
void print_level(WINDOW *w_level, u8 level) {
    mvwprintw(w_level, BORDER_THICKNESS, BORDER_THICKNESS, "%hi", level);
    wnoutrefresh(w_level);
}

// Prints the pause screen
void print_pause(WINDOW *win, Game *game) {
    move(BORDER_THICKNESS, BORDER_THICKNESS);
    for (u8 y = 0; y < (FIELD_Y - FIELD_UM) * game->block_size.y; y++) {
        for (u8 x = 0; x < FIELD_X * game->block_size.x; x++)
            addch(PAUSE_CHAR);
        move(BORDER_THICKNESS + (y + 1), BORDER_THICKNESS);
    }

    border_draw(win, "PAUSED");
}

// Pauses the game
bool pause_game(WINDOW *w_field, Game *game, i16 *ch) {
    print_pause(w_field, game);

    timeout(-1); // wait indefinitely for input
    do {
        *ch = getch();
        if (*ch == CH_QUIT)
            return false;
    } while(*ch != CH_PAUSE); 

    // Redraw field for a second
    werase(w_field);
    field_draw(w_field, game->block_size, game);
    tm_draw_ghost(w_field, game->block_size, game, &game->tm_field);
    tm_draw(w_field, game->block_size, &game->tm_field, false);
    border_draw(w_field, WINT_FIELD_PAUSED);
    doupdate();

    timeout(0); // don't wait for input
    sleep(SECONDS_AFTER_PAUSE);

    // Emptying the input queue
    do {
        *ch = getch();
    } while (*ch != ERR);

    return true;
}

// Draws the game to the stdscr
void draw_game(WINDOW *win[WINDOW_NUM], Game *game, u8 *blink_frame) {
    // Don't draw anything when new tetromino is set to enter
    if (game->entry_delay == 0 || game->entry_delay == ENTRY_DELAY) {
        // Clearing the windows 
        // (besides the score and level, which don't have to ever be redrawn)
        for (u8 w = 0; w < WINDOW_NUM - 2; w++)
            werase(win[w]);

        // Drawing
        field_draw(win[WIN_FIELD], game->block_size, game);
        tm_nh_draw(win[WIN_HOLDTM], game->block_size, &game->tm_hold);
        tm_nh_draw(win[WIN_NEXTTM], game->block_size, &game->tm_next);
        print_score(win[WIN_SCORE], game->score);
        print_level(win[WIN_LEVEL], game->level);

        if (!tm_on_floor(game, &game->tm_field)) {
            tm_draw_ghost(win[WIN_FIELD], game->block_size, game, &game->tm_field);
            tm_draw(win[WIN_FIELD], game->block_size, &game->tm_field, false);
            *blink_frame = UINT8_MAX;
        } else { // Blink when on the floor
            if (*blink_frame == UINT8_MAX) // just landed
                *blink_frame = BLINK_FRAMES;
            if (*blink_frame == 0)
                *blink_frame = BLINK_INTERVAL;
            if (*blink_frame <= BLINK_FRAMES) {
                tm_draw_ghost(win[WIN_FIELD], game->block_size, game, &game->tm_field);
                tm_draw(win[WIN_FIELD], game->block_size, &game->tm_field, false);
            }

            *blink_frame = *blink_frame - 1;
        }

        // Borders
        border_draw(win[WIN_FIELD], WINT_FIELD);
        border_draw(win[WIN_HOLDTM], WINT_HOLDTM);
        border_draw(win[WIN_NEXTTM], WINT_NEXTTM);
        border_draw(win[WIN_SCORE], WINT_SCORE);
        border_draw(win[WIN_LEVEL], WINT_LEVEL);
    }

    doupdate();
}
