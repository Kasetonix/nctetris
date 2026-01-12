#include <curses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

#include "utils.h"
#include "win_loc_dim.h"
#include "game.h"
#include "draw.h"

int main() {
    bool run = true;
    i16 ch;
    u8 blink_frame;
    Windim scrdim;
    struct timespec timestamp, sleep_time;

    WINDOW *win[WINDOW_NUM];

    srand(time(NULL));

    init_ncurses();

    Game game = {
        .score = 0,
        .lines_cleared = 0,
        .level = 1,
        .combo = -1,
        .bag = { 0, 1, 2, 3, 4, 5, 6 },
        .bag_index = 0,
        .on_floor = false,
        .swapped = false,
        .entry_delay = 0,
        .gravity_acted = false,
        .paused = false,
    };

    for (u8 y = 0; y < FIELD_Y; y++) {
        for (u8 x = 0; x < FIELD_X; x++) {
            game.field[y][x] = BLACK;
        }
    }

    scrdim = get_scrdim();
    // Scaling the windows if there is enough space
    if (scrdim.cols >= 62 && scrdim.rows >= 42)
        game.block_size = (Vec) { 2, 4 };
    else
        game.block_size = (Vec) { 1, 2 };

    game.tm_next = tm_create_rand(&game);
    game.tm_hold = tm_create_rand(&game);
    game.tm_hold.type = BLACK;
    tm_spawn(&game);

    win[WIN_FIELD]  = create_win(WINLOC_FIELD_Y, WINLOC_FIELD_X, WINDIM_FIELD_Y, WINDIM_FIELD_X);
    win[WIN_NEXTTM] = create_win(WINLOC_NEXTTM_Y, WINLOC_NEXTTM_X, WINDIM_NEXTTM_Y, WINDIM_NEXTTM_X);
    win[WIN_HOLDTM] = create_win(WINLOC_HOLDTM_Y, WINLOC_HOLDTM_X, WINDIM_HOLDTM_Y, WINDIM_HOLDTM_X);
    win[WIN_SCORE]  = create_win(WINLOC_SCORE_Y, WINLOC_SCORE_X, WINDIM_SCORE_Y, WINDIM_SCORE_X);
    win[WIN_LEVEL]  = create_win(WINLOC_LEVEL_Y, WINLOC_LEVEL_X, WINDIM_LEVEL_Y, WINDIM_LEVEL_X);

    clock_gettime(CLOCK_REALTIME, &timestamp);
    while (run) {
        if (!tick(&game, ch))
            run = !run;

        draw_game(&win[0], &game, &blink_frame);

        ch = getch();
        if (ch == CH_PAUSE)
            if (!pause_game(win[WIN_FIELD], &game, &ch))
                run = !run;

        sleep_time = time_to_sleep(timestamp);
        nanosleep(&sleep_time, NULL);
        clock_gettime(CLOCK_REALTIME, &timestamp);
    }

    endwin();
    printf("LEVEL: %hu | SCORE: %u\n", game.level, game.score);
    return 0;
}
