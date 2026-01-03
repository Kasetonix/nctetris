#include <curses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "game.h"

// All tetromino variants saved as arrays of blocks
const Vec TM_BLOCKS[TM_NUM][TM_ORIENT][TM_SIZE] = { // relative (y, x) coordinates
    { // I
        { {1, 0}, {1, 1}, {1, 2}, {1, 3} },
        { {0, 2}, {1, 2}, {2, 2}, {3, 2} },
        { {2, 0}, {2, 1}, {2, 2}, {2, 3} },
        { {0, 1}, {1, 1}, {2, 1}, {3, 1} }
    }, 
    { // O 
        { {0, 0}, {0, 1}, {1, 0}, {1, 1} }, 
        { {0, 0}, {0, 1}, {1, 0}, {1, 1} },
        { {0, 0}, {0, 1}, {1, 0}, {1, 1} },
        { {0, 0}, {0, 1}, {1, 0}, {1, 1} }
    },
    { // T
        { {1, 0}, {1, 1}, {1, 2}, {0, 1} },
        { {0, 1}, {1, 1}, {2, 1}, {1, 2} },
        { {1, 0}, {1, 1}, {1, 2}, {2, 1} },
        { {0, 1}, {1, 1}, {2, 1}, {1, 0} },
    }, 
    { // J 
        { {1, 0}, {1, 1}, {1, 2}, {0, 0} },
        { {0, 1}, {1, 1}, {2, 1}, {0, 2} },
        { {1, 0}, {1, 1}, {1, 2}, {2, 2} },
        { {0, 1}, {1, 1}, {2, 1}, {2, 0} },
    },
    { // L
        { {1, 0}, {1, 1}, {1, 2}, {0, 2} },
        { {0, 1}, {1, 1}, {2, 1}, {2, 2} },
        { {1, 0}, {1, 1}, {1, 2}, {2, 0} },
        { {0, 1}, {1, 1}, {2, 1}, {0, 0} },
    },
    { // S 
        { {1, 0}, {1, 1}, {0, 1}, {0, 2} },
        { {0, 1}, {1, 1}, {1, 2}, {2, 2} },
        { {1, 2}, {1, 1}, {2, 1}, {2, 0} },
        { {2, 1}, {1, 1}, {1, 0}, {0, 0} }
    },
    { // Z
        { {1, 2}, {1, 1}, {0, 1}, {0, 0} },
        { {2, 1}, {1, 1}, {1, 2}, {0, 2} },
        { {1, 0}, {1, 1}, {2, 1}, {2, 2} },
        { {0, 1}, {1, 1}, {1, 0}, {2, 0} }
    }
};

// Gravity level depending on game level
const u8 GRAVITY[LVL_NUM] = {
  // 0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
    50, 48, 46, 44, 42, 40, 38, 36, 34, 32,
  // 10, 11, 12, 13, 14, 15, 16, 17, 18, 19,
    30, 28, 26, 24, 22, 20, 16, 12,  8,  4
};

// Calculates the correct origin for drawing tetrominos in Hold and Next windows
static Vec calc_nh_pos(Vec block_size, Tetromino *tm) {
    Vec nh_pos = { BORDER_THICKNESS, BORDER_THICKNESS };
    Vec tm_min_corner = { TM_SIZE - 1, TM_SIZE - 1 }, tm_max_corner = { 0, 0 };
    u8 margin_h, margin_v;

    // Getting the bounding box of a tetromino
    for (u8 i = 0; i < TM_SIZE; i++) {
        if (tm->block[i].x < tm_min_corner.x)
            tm_min_corner.x = tm->block[i].x;
        if (tm->block[i].x > tm_max_corner.x)
            tm_max_corner.x = tm->block[i].x;
        if (tm->block[i].y < tm_min_corner.y)
            tm_min_corner.y = tm->block[i].y;
        if (tm->block[i].y > tm_max_corner.y)
            tm_max_corner.y = tm->block[i].y;
    }

    // Converting to drawing coordinates
    tm_min_corner.x = tm_min_corner.x * block_size.x;
    tm_min_corner.y = tm_min_corner.y * block_size.y;
    tm_max_corner.x = (tm_max_corner.x + 1) * block_size.x - 1;
    tm_max_corner.y = (tm_max_corner.y + 1) * block_size.y - 1;

    // Calculating the margin size
    margin_h = (block_size.x * TM_SIZE - (tm_max_corner.x - tm_min_corner.x + 1)) / 2;
    margin_v = (block_size.y * TM_SIZE - (tm_max_corner.y - tm_min_corner.y + 1)) / 2;

    nh_pos.x += margin_h - tm_min_corner.x;
    nh_pos.y += margin_v - tm_min_corner.y;

    return nh_pos;
}

// Generates a random tetromino
Tetromino tm_create_rand(Vec block_size) {
    Tetromino tm;
    tm.type = rand() % TM_NUM;
    tm.orientation = 0;
    tm.pos = (Vec) { 0, 0 }; 

    for (u8 i = 0; i < TM_SIZE; i++) {
        tm.block[i] = TM_BLOCKS[tm.type][tm.orientation][i];
    }

    tm.pos_nh = calc_nh_pos(block_size, &tm);
    return tm;
}

// Checks whether a tetromino fits in a given position
static bool tm_fits(Game *game, Tetromino *tm, Vec offset) {
    u8 tm_min_x = TM_SIZE - 1, tm_max_x = 0, tm_min_y = TM_SIZE - 1, tm_max_y = 0;
    Vec moved_block_pos;

    for (u8 i = 0; i < TM_SIZE; i++) {
        if (tm->block[i].x > tm_max_x)
            tm_max_x = tm->block[i].x;
        if (tm->block[i].x < tm_min_x)
            tm_min_x = tm->block[i].x;
        if (tm->block[i].y > tm_max_y)
            tm_max_y = tm->block[i].y;
        if (tm->block[i].y < tm_min_y)
            tm_min_y = tm->block[i].y;
    }

    if (tm->pos.x + offset.x + tm_min_x < 0 ||
        tm->pos.y + offset.y + tm_min_y < 0 ||
        tm->pos.x + offset.x + tm_max_x >= FIELD_X ||
        tm->pos.y + offset.y + tm_max_y >= FIELD_Y )
        return false;

    for (u8 i = 0; i < TM_SIZE; i++) {
        moved_block_pos.x = tm->block[i].x + tm->pos.x + offset.x;
        moved_block_pos.y = tm->block[i].y + tm->pos.y + offset.y;

        if (game->field[moved_block_pos.y][moved_block_pos.x] != BLACK)
            return false;
    }

    return true;
}

// Spawns a next tetromino onto the field
bool tm_spawn(Game *game) {
    game->falling_tm = game->next_tm;
    game->next_tm = tm_create_rand(game->block_size);
    game->falling_tm.pos = (Vec) { 0, FIELD_X / 2 - 1 };
    game->falling_tm.pos_nh = calc_nh_pos(game->block_size, &game->falling_tm);
    game->falling_tm.gravity_timer = GRAVITY[game->lines_cleared / LINES_PER_LEVEL];
    
    if (!tm_fits(game, &game->falling_tm, (Vec) { 0, 0 }))
        return false;
    return true;
}

// Attempts to move a tetromino in a given direction,
// returns true if tetromino was successfully moved
static bool tm_mv(Game *game, Tetromino *tm, Direction dir) {
    switch (dir) {
        case LEFT: 
            if (tm_fits(game, tm, (Vec) { 0, -1 })) {
                tm->pos.x--;
                return true;
            } break;
        case RIGHT: 
            if (tm_fits(game, tm, (Vec) { 0, 1 })) {
                tm->pos.x++;
                return true;
            } break;
        case UP: 
            if (tm_fits(game, tm, (Vec) { -1, 0 })) {
                tm->pos.y--;
                return true;
            } break;
        case DOWN: 
            if (tm_fits(game, tm, (Vec) { 1, 0 })) {
                tm->pos.y++;
                return true;
            } break;
    }

    return false;

}

// Shortcut for moving the currently falling tetromino
static inline bool tm_falling_mv(Game *game, Direction dir) {
    return tm_mv(game, &game->falling_tm, dir);
}

// Sets a tetromino onto the field
static void tm_set(Game *game) {
    Vec block_pos;
    for (u8 i = 0; i < TM_SIZE; i++) {
        block_pos = (Vec) { 
            .x = game->falling_tm.pos.x + game->falling_tm.block[i].x,
            .y = game->falling_tm.pos.y + game->falling_tm.block[i].y
        };
        game->field[block_pos.y][block_pos.x] = game->falling_tm.type;
    }
}

// Exchanges the currently falling tetromino with the held one
static void tm_hold(Game *game) {
    Vec offset;
    Tetromino tm_tmp; 

    offset = game->falling_tm.pos;
    offset.x -= game->held_tm.pos.x;
    offset.y -= game->held_tm.pos.y;

    if (!tm_fits(game, &game->held_tm, offset))
        return;

    // not holding a tetromino
    if (game->held_tm.type == BLACK) { 
        game->held_tm = game->next_tm;
        game->next_tm = tm_create_rand(game->block_size);
    } 

    game->held_tm.pos = game->falling_tm.pos;
    game->held_tm.gravity_timer = game->falling_tm.gravity_timer;
    tm_tmp = game->falling_tm;
    game->falling_tm = game->held_tm;
    game->held_tm = tm_tmp;

}

// Rotates the falling tetromino clockwise
static void tm_rotate(Game *game) {
    Tetromino tm_tmp = game->falling_tm;
    tm_tmp.orientation = (tm_tmp.orientation + 1) % TM_ORIENT;
    for (u8 i = 0; i < TM_SIZE; i++)
        tm_tmp.block[i] = TM_BLOCKS[tm_tmp.type][tm_tmp.orientation][i];
    tm_tmp.pos_nh = calc_nh_pos(game->block_size, &tm_tmp);

    if (tm_fits(game, &tm_tmp, (Vec) { 0, 0 })) {
        game->falling_tm = tm_tmp;
    } else if (tm_mv(game, &tm_tmp, LEFT)) {
        game->falling_tm = tm_tmp;
    } else if (tm_mv(game, &tm_tmp, RIGHT)) {
        game->falling_tm = tm_tmp;
    }
}

// Checks if a line on a field is full
static bool is_line_full(Game *game, u8 line) {
    for (u8 x = 0; x < FIELD_X; x++)
        if (game->field[line][x] == BLACK)
            return false;
    return true;
}

static void remove_line(Game *game, u8 line) {
    for (i8 y = line; y > 0; y--)
        for (u8 x = 0; x < FIELD_X; x++)
            game->field[y][x] = game->field[y-1][x];

    for (u8 x = 0; x < FIELD_X; x++)
        game->field[0][x] = BLACK;
}

static void clear_lines(Game *game) {
    u8 lines_cleared = 0;
    u16 multiplier = 0;

    for (u8 line = 0; line < FIELD_Y; line++) {
        if (is_line_full(game, line)) {
            remove_line(game, line);
            lines_cleared++;
            line++;
        }
    }

    if (!is_line_full(game, FIELD_Y - 1)) {
        switch (lines_cleared) {
            case 1: multiplier = 100; break;
            case 2: multiplier = 300; break;
            case 3: multiplier = 500; break;
            case 4: multiplier = 800; break;
        }
    } else { // Perfect clear
        switch (lines_cleared) {
            case 1: multiplier = 800; break;
            case 2: multiplier = 1200; break;
            case 3: multiplier = 1800; break;
            case 4: multiplier = 3200; break;
        }
    }

    game->score += multiplier * (game->lines_cleared / LINES_PER_LEVEL + 1);
    game->lines_cleared += lines_cleared;
}

static void hard_drop(Game *game) {
    u8 init_y, height;

    init_y = game->falling_tm.pos.y;
    while (tm_falling_mv(game, DOWN));
    height = game->falling_tm.pos.y - init_y;
    game->score += 2 * height;
}

// Draws a singular block
static void block_draw(WINDOW *win, Vec block_size, Vec pos, u8 color, bool preview) {
    if (color == BLACK)
        return;
    if (pos.x < 0 || pos.y < 0)
        return;

    if (!preview) {
        for (u8 y = 0; y < block_size.y; y++) {
            wmove(win, pos.y + y, pos.x);
            for (u8 x = 0; x < block_size.x; x++) {
                waddch(win, DRAW_CHAR | A_REVERSE | COLOR_PAIR(color));
            }
        }
    } else {
        for (u8 y = 0; y < block_size.y; y++) {
            wmove(win, pos.y + y, pos.x);
            for (u8 x = 0; x < block_size.x; x++) {
                waddch(win, PREV_CHAR | COLOR_PAIR(color));
            }
        }
        
    }

    wnoutrefresh(win);
}

// Draws a tetromino
void tm_draw(WINDOW *win, Vec block_size, Tetromino *tm, bool preview) {
    Vec drawing_pos;
    for (u8 i = 0; i < TM_SIZE; i++) {
        drawing_pos.y = block_size.y * (tm->pos.y + tm->block[i].y - FIELD_UM) + BORDER_THICKNESS;
        drawing_pos.x = block_size.x * (tm->pos.x + tm->block[i].x) + BORDER_THICKNESS;
        block_draw(win, block_size, drawing_pos, tm->type, preview);
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

// Draws a preview of a tetromino; it's final position when dropped
void tm_draw_preview(WINDOW *win, Vec block_size, Game *game, Tetromino *tm) {
    Tetromino tm_preview = *tm;
    while (tm_fits(game, &tm_preview, (Vec) { 1, 0 }))
        tm_preview.pos.y++;
    tm_draw(win, block_size,  &tm_preview, true);
}

// Draws the entire game field
void field_draw(WINDOW *w_field, Vec block_size, Game *game) {
    Vec pos = { BORDER_THICKNESS, BORDER_THICKNESS };
    bool prev = false;

    for (u8 y = FIELD_UM; y < FIELD_Y; y++) {
        if (is_line_full(game, y))
            prev = true;
        for (u8 x = 0; x < FIELD_X; x++) {
            block_draw(w_field, block_size, pos, game->field[y][x], prev);
            pos.x += block_size.x;
        }
        pos.x = 1;
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

// Performs the game logic in a given frame
bool tick(Game *game, u16 ch) {
    u8 init_tm_falling_y, height;
    game->frame++;

    // Setting the correct gravity timer if a tetromino is on the floor
    if (game->gravity_acted && !tm_fits(game, &game->falling_tm, (Vec) { 1, 0 }))
        game->falling_tm.gravity_timer = FRAMES_BEFORE_SET; 

    // Input handling
    switch (ch) {
        case KEY_LEFT:  tm_falling_mv(game, LEFT); break; 
        case KEY_RIGHT: tm_falling_mv(game, RIGHT); break;
        case KEY_UP:    tm_rotate(game); break;
        case 'z':       tm_hold(game); break;
        case 'q':       return false; break;

        case 'x':
            hard_drop(game);
            tm_set(game);
            if (!tm_spawn(game)) { 
                sleep(1); 
                return false; 
            }
            break;

        case KEY_DOWN:  
            if (!tm_falling_mv(game, DOWN)) 
                tm_set(game); 
            if (game->gravity_acted && !tm_fits(game, &game->falling_tm, (Vec) { 1, 0 }))
                game->falling_tm.gravity_timer = FRAMES_BEFORE_SET;
            game->score++;
            break;
    }

    // Handling gravity
    game->falling_tm.gravity_timer--;
    if (game->falling_tm.gravity_timer == 0) {
        game->falling_tm.gravity_timer = GRAVITY[game->lines_cleared / LINES_PER_LEVEL];
        game->gravity_acted = true;
        if (!tm_falling_mv(game, DOWN)) {
            tm_set(game);
            if (!tm_spawn(game)) { 
                sleep(1); 
                return false; 
            }
        }
    } else {
        game->gravity_acted = false;
    }

    clear_lines(game);

    return true;
}
