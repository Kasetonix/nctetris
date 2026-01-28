# NCTetris
A Tetris clone written in C (C99) for POSIX-compliant systems. With the exception of T-spins it should be fairly compliant with recent Tetris guidelines (gameplay-wise). Only depends on ncurses, C standard library and POSIX extensions.

# Building
## `ncurses` installation 
- Arch Linux: 
```sh
sudo pacman -S ncurses
```

- Debian/Ubuntu:
```sh
sudo apt install libncurses-dev
```

- Fedora:
```sh
sudo yum install ncurses-devel
```

## Compilation
- `make`:
    ```sh
    make clean
    ```

- Manual:
    Compile all of the source files to object files with `-std=c99` flag and link them with the ncurses library.
    ```bash
    for src in *.c; do cc -c -std=gnu99 "$src"; done && \
    cc *.c -lncurses -o tetris && \
    rm *.o
    ```

# Running
After compilation there should be an executable `tetris` file in the root of this repo; run it and enjoy!

# Controls
- `←` - Move left
- `→` - Move right
- `↑` - Rotate clockwise 
- `z` - Rotate counterclockwise
- `↓` - Soft drop
- `␣` - Hard drop
- `c` - Hold a tetromino
- `q` - quit the game
- `p` - pause the game

# Used resources
- [Tetris wiki](https://tetris.wiki/), especially the [Tetris Guideline](https://tetris.wiki/Tetris_Guideline) page - for looking up specific game rules,
- [NCURSES-Programming-HOWTO](https://tldp.org/HOWTO/NCURSES-Programming-HOWTO/) by Pradeep Padala - for learning the basics of the `ncurses` library,
- `man` pages for various `ncurses` functions - for detailed usage of `ncurses` functions.
