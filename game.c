#include <curses.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include "game.h"

// All tetromino variants saved as arrays of blocks
const Vec TM_BLOCKS[TM_NUM][TM_ORIENT][TM_SIZE] = { // relative (y, x) coordinates
    { // O 
        { {0, 0}, {0, 1}, {1, 0}, {1, 1} }, 
        { {0, 0}, {0, 1}, {1, 0}, {1, 1} },
        { {0, 0}, {0, 1}, {1, 0}, {1, 1} },
        { {0, 0}, {0, 1}, {1, 0}, {1, 1} }
    },
    { // Z
        { {1, 2}, {1, 1}, {0, 1}, {0, 0} },
        { {2, 1}, {1, 1}, {1, 2}, {0, 2} },
        { {1, 0}, {1, 1}, {2, 1}, {2, 2} },
        { {0, 1}, {1, 1}, {1, 0}, {2, 0} }
    },
    { // S 
        { {1, 0}, {1, 1}, {0, 1}, {0, 2} },
        { {0, 1}, {1, 1}, {1, 2}, {2, 2} },
        { {1, 2}, {1, 1}, {2, 1}, {2, 0} },
        { {2, 1}, {1, 1}, {1, 0}, {0, 0} }
    },
    { // L
        { {1, 0}, {1, 1}, {1, 2}, {0, 2} },
        { {0, 1}, {1, 1}, {2, 1}, {2, 2} },
        { {1, 0}, {1, 1}, {1, 2}, {2, 0} },
        { {0, 1}, {1, 1}, {2, 1}, {0, 0} },
    },
    { // J 
        { {1, 0}, {1, 1}, {1, 2}, {0, 0} },
        { {0, 1}, {1, 1}, {2, 1}, {0, 2} },
        { {1, 0}, {1, 1}, {1, 2}, {2, 2} },
        { {0, 1}, {1, 1}, {2, 1}, {2, 0} },
    },
    { // T
        { {1, 0}, {1, 1}, {1, 2}, {0, 1} },
        { {0, 1}, {1, 1}, {2, 1}, {1, 2} },
        { {1, 0}, {1, 1}, {1, 2}, {2, 1} },
        { {0, 1}, {1, 1}, {2, 1}, {1, 0} },
    }, 
    { // I
        { {1, 0}, {1, 1}, {1, 2}, {1, 3} },
        { {0, 2}, {1, 2}, {2, 2}, {3, 2} },
        { {2, 0}, {2, 1}, {2, 2}, {2, 3} },
        { {0, 1}, {1, 1}, {2, 1}, {3, 1} }
    }, 
};

// Gravity level depending on game level
const u8 GRAVITY[LVL_NUM] = {
//   0   1   2   3   4   5   6   7   8   9
    50, 48, 46, 44, 42, 40, 38, 36, 34, 32,
//  10  11  12  13  14  15  16  17  18  19
    30, 28, 26, 24, 22, 20, 16, 12,  8,  4
};

// Calculates the bounding box for tetrominoes
static BoundingBox calc_bbox(Tetromino *tm) {
    BoundingBox bbox =  { 
        .top = TM_SIZE - 1,
        .left = TM_SIZE - 1,
        .right = 0,
        .bottom = 0
    };

    for (u8 i = 0; i < TM_SIZE; i++) {
        if (tm->block[i].x < bbox.left)
            bbox.left = tm->block[i].x;
        if (tm->block[i].x > bbox.right)
            bbox.right = tm->block[i].x;
        if (tm->block[i].y < bbox.top)
            bbox.top = tm->block[i].y;
        if (tm->block[i].y > bbox.bottom)
            bbox.bottom = tm->block[i].y;
    }

    return bbox;
}

// Calculates the correct origin for drawing tetrominos in Hold and Next windows
static Vec calc_nh_pos(Vec block_size, Tetromino *tm) {
    Vec nh_pos = { BORDER_THICKNESS, BORDER_THICKNESS };
    u8 margin_h, margin_v;

    // Calculating the margin size in drawing coordinates
    margin_h = (TM_SIZE - (tm->bbox.right - tm->bbox.left + 1)) * block_size.x / 2;
    margin_v = (TM_SIZE - (tm->bbox.bottom - tm->bbox.top + 1)) * block_size.y / 2;

    nh_pos.x += margin_h - tm->bbox.left * block_size.x;
    nh_pos.y += margin_v - tm->bbox.top * block_size.y;

    return nh_pos;
}

static void tm_get_data(Vec block_size, Tetromino *tm, Vec pos, u8 type, u8 orientation) {
    tm->type = type;
    tm->orientation = orientation;
    tm->pos = (Vec) { pos.y, pos.x };

    for (u8 i = 0; i < TM_SIZE; i++) {
        tm->block[i] = TM_BLOCKS[tm->type][tm->orientation][i];
    }

    tm->bbox = calc_bbox(tm);
    tm->pos_nh = calc_nh_pos(block_size, tm);
}

// Generates a random tetromino
Tetromino tm_create_rand(Vec block_size) {
    Tetromino tm;
    tm_get_data(block_size, &tm, (Vec) { 0, 0 }, rand() % TM_NUM, 0);
    tm.pos.x = (FIELD_X - (tm.bbox.right - tm.bbox.left + 1)) / 2;
    return tm;
}

// Returns a rotated tetromino without any checks
static Tetromino tm_rotated(Vec block_size, Tetromino *tm, bool clockwise) {
    Tetromino tm_r;
    u8 orientation = (tm->orientation + (clockwise ? 1 : TM_ORIENT - 1)) % TM_ORIENT;
    tm_get_data(block_size, &tm_r, tm->pos, tm->type, orientation);
    return tm_r; 
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

// Checks if the falling tetromino is on the floor
bool tm_on_floor(Game *game, Tetromino *tm) {
    return !tm_fits(game, tm, (Vec) { 1, 0 });
}

// Spawns a next tetromino onto the field
bool tm_spawn(Game *game) {
    game->tm_field = game->tm_next;
    game->tm_next = tm_create_rand(game->block_size);
    game->gravity_timer = GRAVITY[game->lines_cleared / LINES_PER_LEVEL];
    game->floor_counter = LOCKDOWN_FRAMES;
    
    if (!tm_fits(game, &game->tm_field, (Vec) { 0, 0 }))
        return false;
    return true;
}

// Attempts to move a tetromino in a given direction,
// returns true if tetromino was successfully moved
static bool tm_mv(Game *game, Tetromino *tm, Direction dir) {
    bool moved = false;
    switch (dir) {
        case LEFT: 
            if (tm_fits(game, tm, (Vec) { 0, -1 })) {
                tm->pos.x--;
                moved = true;
            } break;
        case RIGHT: 
            if (tm_fits(game, tm, (Vec) { 0, 1 })) {
                tm->pos.x++;
                moved = true;
            } break;
        case UP: 
            if (tm_fits(game, tm, (Vec) { -1, 0 })) {
                tm->pos.y--;
                moved = true;
            } break;
        case DOWN: 
            if (tm_fits(game, tm, (Vec) { 1, 0 })) {
                tm->pos.y++;
                moved = true;
            } break;
    }


    return moved;
}

// Sets a tetromino onto the field
static void tm_lock(Game *game) {
    Vec block_pos;
    for (u8 i = 0; i < TM_SIZE; i++) {
        block_pos = (Vec) { 
            .x = game->tm_field.pos.x + game->tm_field.block[i].x,
            .y = game->tm_field.pos.y + game->tm_field.block[i].y
        };
        game->field[block_pos.y][block_pos.x] = game->tm_field.type;
    }
    game->entry_delay = ENTRY_DELAY;
    game->tm_field.type = BLACK;
}

// Handles lock down delay for field tetromino,
// returns false if the tetromino has been locked down
static bool tm_handle_ldd(Game *game) {
    if (tm_on_floor(game, &game->tm_field)) {
        if (game->floor_counter != 0) {
            game->floor_counter--;
            game->gravity_timer = LOCKDOWN_FRAMES;
        } else {
            tm_lock(game);
            return false;
        }
    }

    return true;
}

// Shortcut for moving the field tetromino
static inline bool tmf_mv(Game *game, Direction dir) {
    tm_handle_ldd(game);
    return tm_mv(game, &game->tm_field, dir);
}

// Exchanges the field tetromino with the held one
static void tm_hold(Game *game) {
    Vec offset;
    Tetromino tm_tmp; 

    offset = game->tm_field.pos;
    offset.x -= game->tm_hold.pos.x;
    offset.y -= game->tm_hold.pos.y;

    if (!tm_fits(game, &game->tm_hold, offset))
        return;

    // not holding a tetromino
    if (game->tm_hold.type == BLACK) { 
        game->tm_hold = game->tm_next;
        game->tm_next = tm_create_rand(game->block_size);
    } 

    game->tm_hold.pos = game->tm_field.pos;

    tm_tmp = game->tm_field;
    game->tm_field = game->tm_hold;
    game->tm_hold = tm_tmp;
}

// Rotates the field tetromino clockwise
static void tm_rotate(Game *game, bool clockwise) {
    tm_handle_ldd(game);
    if (game->tm_field.type == TM_O) {
        game->tm_field.orientation++;
        return;
    }

    Tetromino tm_tmp = tm_rotated(game->block_size, &game->tm_field, clockwise);

    if (tm_fits(game, &tm_tmp, (Vec) { 0, 0 })) {
        game->tm_field = tm_tmp;
    } else if (tm_mv(game, &tm_tmp, LEFT)) {
        game->tm_field = tm_tmp;
    } else if (tm_mv(game, &tm_tmp, RIGHT)) {
        game->tm_field = tm_tmp;
    }
}

// Checks if a line on a field is full
static bool is_line_full(Game *game, u8 line) {
    for (u8 x = 0; x < FIELD_X; x++)
        if (game->field[line][x] == BLACK)
            return false;
    return true;
}

// Checks if a line on a field is empty 
static bool is_line_empty(Game *game, u8 line) {
    for (u8 x = 0; x < FIELD_X; x++)
        if (game->field[line][x] != BLACK)
            return false;
    return true;
}

// Removes a line by moving all of the lines above one block down
static void remove_line(Game *game, u8 removed_line) {
    for (u8 y = removed_line; y > 0; y--)
        for (u8 x = 0; x < FIELD_X; x++)
            game->field[y][x] = game->field[y-1][x];

    for (u8 x = 0; x < FIELD_X; x++)
        game->field[0][x] = BLACK;
}

// Awards points based on how many lines were cleared
static void award_points(Game *game, u8 lines_cleared) {
    u16 multiplier = 0;
    // Lowest line being empty means that the whole field is
    // empty - perfect clear
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
}

// Clears all full lines and awards points
static void clear_lines(Game *game) {
    u8 lines_cleared = 0;
    u8 removed_line;

    for (u8 line = 0; line < FIELD_Y; line++) {
        if (is_line_full(game, line)) {
            lines_cleared++;
            remove_line(game, line);
            if (lines_cleared == 4)
                break;
        }
    }

    award_points(game, lines_cleared);
    game->lines_cleared += lines_cleared;
}

// Drops the field tetromino to the ground and awards points
static void hard_drop(Game *game) {
    u8 init_y, height;

    init_y = game->tm_field.pos.y;
    while (tmf_mv(game, DOWN));
    height = game->tm_field.pos.y - init_y;
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
    if (tm->type == BLACK)
        return;

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

// Draws a preview of a tetromino; it's final position when hard dropped
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
        // if (is_line_full(game, y))
        //     prev = true;
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
    if (game->entry_delay > 1) {
        game->entry_delay--;
        return true;
    } else if (game->entry_delay == 1) {
        game->entry_delay--;
        if (!tm_spawn(game)) { 
            sleep(1); 
            return false; 
        }
    }

    if (!tm_on_floor(game, &game->tm_field))
        game->floor_counter = FLOOR_MOVES; 

    // Input handling
    switch (ch) {
        case CH_MV_LEFT:    tmf_mv(game, LEFT); break; 
        case CH_MV_RIGHT:   tmf_mv(game, RIGHT); break;
        case CH_ROTATE_CW:  tm_rotate(game, true); break;
        case CH_ROTATE_CCW: tm_rotate(game, false); break;
        case CH_HOLD:       tm_hold(game); break;
        case CH_QUIT:       return false; break;
        case CH_HARD_DROP:  hard_drop(game); tm_lock(game); break;
        
        case CH_SOFT_DROP:  
            if (tmf_mv(game, DOWN)) {
                game->score++;
                game->gravity_acted = true;
            } break;
    }

    // Setting the correct gravity timer if a tetromino just fell
    if (game->gravity_acted && tm_on_floor(game, &game->tm_field))
        game->gravity_timer = LOCKDOWN_FRAMES; 

    // Handling gravity
    if (game->gravity_timer == 0) {
        game->gravity_timer = GRAVITY[game->lines_cleared / LINES_PER_LEVEL];
        game->gravity_acted = true;
        if (!tmf_mv(game, DOWN))
            tm_lock(game);
    } else {
        game->gravity_acted = false;
        game->gravity_timer--;
    }

    clear_lines(game);

    return true;
}
