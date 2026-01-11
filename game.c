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
        { {0, 1}, {1, 1}, {2, 1}, {3, 1} },
    }, 
};

// Gravity level depending on game level (for 60FPS)
const u8 GRAVITY[GRAVITY_ARR_SIZE] = {
//   0   1   2   3   4   5   6   7   8   9
    48, 44, 38, 34, 28, 24, 18, 14, 12, 10,
//  10  11  12  13  14  15  16  17  18  19  
    10,  8,  8,  6,  6,  6,  4,  4,  4,  4
};

// Wall kick offset array for Z, S, L, J and T Tetrominos
const Vec WALL_KICK[TM_ROT_DIRS][TM_ORIENT][WK_TESTS] = {
    { // Clockwise 
        { { 0,-1}, { 1,-1}, {-2, 0}, {-2,-1} }, // 3 -> 0
        { { 0 -1}, {-1,-1}, { 2, 0}, { 2,-1} }, // 0 -> 1
        { { 0, 1}, { 1, 1}, {-2, 0}, {-2, 1} }, // 1 -> 2
        { { 0, 1}, {-1, 1}, { 2, 0}, { 2, 1} }, // 2 -> 3
    },
    { // Counter-Clockwise
        { { 0, 1}, { 1, 1}, {-2, 0}, {-2, 1} }, // 1 -> 0
        { { 0,-1}, {-1,-1}, { 2, 0}, { 2,-1} }, // 2 -> 1
        { { 0,-1}, { 1,-1}, {-2, 0}, {-2,-1} }, // 3 -> 2
        { { 0, 1}, {-1, 1}, { 2, 0}, { 2, 1} }, // 0 -> 3
    }
};

// Wall kick offset array for I Tetrominos
const Vec WALL_KICK_I[TM_ROT_DIRS][TM_ORIENT][WK_TESTS] = {
    { // Clockwise 
        { { 0, 1}, { 0,-2}, { 2, 1}, {-1,-2} }, // 3 -> 0
        { { 0,-2}, { 0, 1}, { 1,-2}, {-2, 1} }, // 0 -> 1
        { { 0,-1}, { 0, 2}, {-2,-1}, { 1, 2} }, // 1 -> 2
        { { 0, 2}, { 0,-1}, {-1, 2}, { 2,-1} }, // 2 -> 3
    },
    { // Counter-Clockwise
        { { 0, 2}, { 0,-1}, {-1, 2}, { 2,-1} }, // 1 -> 0
        { { 0, 1}, { 0,-2}, { 2, 1}, {-1,-2} }, // 2 -> 1
        { { 0,-2}, { 0, 1}, { 1,-2}, {-2, 1} }, // 3 -> 2
        { { 0,-1}, { 0, 2}, {-2,-1}, { 1, 2} }, // 0 -> 3
    }
};

// Returns a correct gravity value for a level
inline static u8 gravity(u8 level) {
    return (u8) ((level <= GRAVITY_ARR_SIZE ? GRAVITY[level-1] : 2) * (FRAMERATE / (60.0)));
}

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

// Inserts data into a tetromino struct
static void tm_insert_data(Vec block_size, Tetromino *tm, Vec pos, Tm_Type type, u8 orientation) {
    tm->type = type;
    tm->orientation = orientation;
    tm->pos = (Vec) { pos.y, pos.x };

    for (u8 i = 0; i < TM_SIZE; i++) {
        tm->block[i] = TM_BLOCKS[tm->type][tm->orientation][i];
    }

    tm->bbox = calc_bbox(tm);
    tm->pos_nh = calc_nh_pos(block_size, tm);
}

// Centers a tetromino
inline static void tm_center(Tetromino *tm) {
    tm->pos.x = (FIELD_X - (tm->bbox.right - tm->bbox.left + 1)) / 2 - tm->bbox.left;
}

// Shuffles a randomizer bag using Fisher-Yates shuffle
static void shuffle_bag(Game *game) {
    u8 i, j;
    Tm_Type tmp;
    for (i = 1; i < BAG_SIZE; i++) {
        j = rand() % (i + 1);
        tmp = game->bag[i];
        game->bag[i] = game->bag[j];
        game->bag[j] = tmp;
    }
}

// Returns a tetromino type from the bag randomizer
static Tm_Type tm_rand(Game *game) {
    Tm_Type val;

    if (game->bag_index == 0)
        shuffle_bag(game);

    val = game->bag[game->bag_index];
    game->bag_index = (game->bag_index + 1) % BAG_SIZE;
    return val;
}

// Generates a random tetromino
Tetromino tm_create_rand(Game *game) {
    Tetromino tm;
    tm_insert_data(game->block_size, &tm, (Vec) { 0, 0 }, tm_rand(game), 0);
    tm_center(&tm);
    return tm;
}

// Returns a rotated tetromino without any checks
static Tetromino tm_rotated(Game *game, Tetromino *tm, bool clockwise) {
    Tetromino tm_r;
    u8 orientation = (tm->orientation + (clockwise ? 1 : TM_ORIENT - 1)) % TM_ORIENT;
    tm_insert_data(game->block_size, &tm_r, tm->pos, tm->type, orientation);
    return tm_r; 
}

// Checks whether a tetromino fits in a given position
bool tm_fits(Game *game, Tetromino *tm, Vec offset) {
    Vec moved_block_pos;

    if (tm->pos.x + offset.x + tm->bbox.left < 0 ||
        tm->pos.y + offset.y + tm->bbox.top < 0 ||
        tm->pos.x + offset.x + tm->bbox.right >= FIELD_X ||
        tm->pos.y + offset.y + tm->bbox.bottom >= FIELD_Y )
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
    game->tm_next = tm_create_rand(game);
    game->gravity_timer = 0;
    game->floor_counter = FLOOR_MOVES;
    game->floor_timer = LOCKDOWN_FRAMES;
    game->swapped = false;
    
    if (!tm_fits(game, &game->tm_field, (Vec) { 0, 0 }))
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
        case DOWN: 
            if (tm_fits(game, tm, (Vec) { 1, 0 })) {
                tm->pos.y++;
                return true;
            } break;
        case UP: 
            if (tm_fits(game, tm, (Vec) { -1, 0 })) {
                tm->pos.y--;
                return true;
            } break;
    }

    return false;
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
            game->floor_timer = LOCKDOWN_FRAMES;
        } else {
            tm_lock(game);
            return false;
        }
    }

    return true;
}

// Shortcut for moving the field tetromino
static bool tmf_mv(Game *game, Direction dir) {
    bool moved;
    tm_handle_ldd(game);
    moved = tm_mv(game, &game->tm_field, dir);
    if (moved && (dir == LEFT || dir == RIGHT) && tm_on_floor(game, &game->tm_field))
        game->floor_timer = LOCKDOWN_FRAMES;

    return moved;
}

// Exchanges the field tetromino with the held one
static void tm_hold(Game *game) {
    if (game->swapped)
        return;

    Tetromino tm_tmp; 

    if (!tm_fits(game, &game->tm_hold, (Vec) { 0, 0 }))
        return;

    // not holding a tetromino
    if (game->tm_hold.type == BLACK) { 
        game->tm_hold = game->tm_next;
        game->tm_next = tm_create_rand(game);
    } 

    tm_tmp = game->tm_field;
    game->tm_field = game->tm_hold;
    game->tm_hold = tm_tmp;

    game->tm_hold.pos.y = 0;
    tm_center(&game->tm_hold);
    game->swapped = true;
}

// Attempts wall kicks and returns true on fit
static bool wall_kick(Game *game, Tetromino *tm, bool clockwise, const Vec (*wka)[TM_ROT_DIRS][TM_ORIENT][WK_TESTS]) {
    Vec offset;
    for (u8 i = 0; i < WK_TESTS; i++) {
        offset = *wka[clockwise? 0 : 1][tm->orientation][i];
        if (tm_fits(game, tm, offset)) {
            tm->pos.y += offset.y;
            tm->pos.x += offset.x;
            return true;
        }
    }

    return false;
}

// Rotates the field tetromino clockwise
static void tm_rotate(Game *game, bool clockwise) {
    tm_handle_ldd(game);
    if (game->tm_field.type == TM_O) {
        game->tm_field.orientation = (game->tm_field.orientation + 1) % TM_ORIENT;
        return;
    }

    Tetromino tm_tmp = tm_rotated(game, &game->tm_field, clockwise);
    if (tm_fits(game, &tm_tmp, (Vec) { 0, 0 }))
        game->tm_field = tm_tmp;
    else if (wall_kick(game, &tm_tmp, clockwise, tm_tmp.type == TM_I? &WALL_KICK_I : &WALL_KICK))
        game->tm_field = tm_tmp;
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

    game->score += multiplier * game->level;
    if (game->combo > 0)
        game->score += 50 * game->combo * game->level;
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

    if (lines_cleared == 0) {
        game->combo = -1;
        return;
    }

    award_points(game, lines_cleared);
    game->lines_cleared += lines_cleared;
    game->level = game->lines_cleared / LINES_PER_LEVEL + 1;
}

// Drops the field tetromino to the ground and awards points
static void hard_drop(Game *game) {
    u8 init_y, height;

    init_y = game->tm_field.pos.y;
    while (tmf_mv(game, DOWN));
    height = game->tm_field.pos.y - init_y;
    game->score += 2 * height;
}

// Performs the game logic in a given frame
bool tick(Game *game, i16 ch) {
    // handling the entry delay
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

    // resetting the floor vars when in the air
    if (!tm_on_floor(game, &game->tm_field)) {
        game->floor_counter = FLOOR_MOVES;
        game->floor_timer = LOCKDOWN_FRAMES;
    }

    // input handling
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
                game->gravity_timer = gravity(game->level);
            } break;
    }

    // lowering the floor timer when on the floor 
    if (tm_on_floor(game, &game->tm_field)) {
        game->floor_timer--;
        game->gravity_timer = gravity(game->level); 
    }

    // locking the piece after the floor timer runs out
    if (game->floor_timer == 0) {
        tm_lock(game);
    }

    // handling gravity
    if (game->gravity_timer == 0) {
        game->gravity_timer = gravity(game->level);
        game->gravity_acted = true;
        if (!tmf_mv(game, DOWN))
            tm_lock(game);
    } else {
        game->gravity_acted = false;
    }
    game->gravity_timer--;

    // clearing lines
    clear_lines(game);

    return true;
}
