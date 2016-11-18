#include <curses.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

struct cols {
    int col;
    int R;
    int G;
    int B;
};

#define DEFAULTCOL 6
#define DEFAULTBGCOL -1

struct cols parse_color(char *col) {
    struct cols ret;

    if (col[0] == '#') {
        if (can_change_color() != TRUE) {
            cleanup_terminal_ncurses();
            fprintf(stderr,
                    "Your terminal can not change color definitions, please "
                    "use one of the predefined colors.\n");
            exit(EXIT_FAILURE);
        }

        // Set to -2 to indicate color_redefinition
        ret.col = -2;
        ++col;
        sscanf(col, "%02x%02x%02x", &ret.R, &ret.G, &ret.B);
        return ret;
    } else {
        // using predefined colors
        ret.col = 0;

        return ret;
    }
}

int init_terminal_ncurses(char *color, char *bcolor, int predefcol,
                          int predefbgcol) {
    struct cols col, bgcol;
    initscr();
    curs_set(0);
    timeout(0);
    noecho();
    start_color();
    use_default_colors();

    double magic = 1000 / 255.0;
    int colp = 0, bgcolp = 0;

    col = parse_color(color);
    bgcol = parse_color(bcolor);
    if (col.col == -2) {
        init_color(1, (int)(col.R * magic), (int)(col.G * magic),
                   (int)(col.B * magic));
    }
    if (bgcol.col == -2) {
        init_color(2, (int)(bgcol.R * magic), (int)(bgcol.G * magic),
                   (int)(bgcol.B * magic));
    }

    switch (col.col) {
    case -2:
        colp = 1;
        break;
    case -1:
        colp = DEFAULTCOL;
        break;
    default:
        colp = predefcol;
    }

    switch (bgcol.col) {
    case -2:
        bgcolp = 2;
        break;
    case -1:
        bgcolp = DEFAULTBGCOL;
        break;
    default:
        bgcolp = predefbgcol;
    }

    init_pair(1, colp, bgcolp);
    if (bgcolp != -1)
        bkgd(COLOR_PAIR(1));
    attron(COLOR_PAIR(1));
    //	attron(A_BOLD);
    return 0;
}

void get_terminal_dim_ncurses(int *w, int *h) {
    getmaxyx(stdscr, *h, *w);
    clear(); // clearing in case of resieze
}

int draw_terminal_ncurses(int tty, int h, int w, int bars, int bw, int bs,
                          int rest, int f[200], int flastd[200]) {
    int i, n, q;
    const wchar_t *bh[] = {L"\u2581", L"\u2582", L"\u2583", L"\u2584",
                           L"\u2585", L"\u2586", L"\u2587", L"\u2588"};

    // output: check if terminal has been resized
    if (!tty) {
        if (LINES != h || COLS != w) {
            return -1;
        }
    }

    h = h - 1;

    for (i = 0; i < bars; i++) {
        if (f[i] > flastd[i]) { // higher then last one
            if (tty)
                for (n = flastd[i] / 8; n < f[i] / 8; n++)
                    for (q = 0; q < bw; q++)
                        mvprintw((h - n), i * bw + q + i * bs + rest, "%d", 8);
            else
                for (n = flastd[i] / 8; n < f[i] / 8; n++)
                    for (q = 0; q < bw; q++)
                        mvaddwstr((h - n), i * bw + q + i * bs + rest, bh[7]);
            if (f[i] % 8 != 0) {
                if (tty)
                    for (q = 0; q < bw; q++)
                        mvprintw((h - n), i * bw + q + i * bs + rest, "%d",
                                 (f[i] % 8));
                else
                    for (q = 0; q < bw; q++)
                        mvaddwstr((h - n), i * bw + q + i * bs + rest,
                                  bh[(f[i] % 8) - 1]);
            }
        } else if (f[i] < flastd[i]) { // lower then last one
            for (n = f[i] / 8; n < flastd[i] / 8 + 1; n++)
                for (q = 0; q < bw; q++)
                    mvaddstr((h - n), i * bw + q + i * bs + rest, " ");
            if (f[i] % 8 != 0) {
                if (tty)
                    for (q = 0; q < bw; q++)
                        mvprintw((h - f[i] / 8), i * bw + q + i * bs + rest,
                                 "%d", (f[i] % 8));
                else
                    for (q = 0; q < bw; q++)
                        mvaddwstr((h - f[i] / 8), i * bw + q + i * bs + rest,
                                  bh[(f[i] % 8) - 1]);
            }
        }
        flastd[i] = f[i]; // memmory for falloff func
    }

    refresh();
    return 0;
}

// general: cleanup
void cleanup_terminal_ncurses(void) {
    echo();
    system("setfont  >/dev/null 2>&1");
    system(
        "setfont /usr/share/consolefonts/Lat2-Fixed16.psf.gz  >/dev/null 2>&1");
    system("setterm -blank 10");
    init_color(1, 1000, 0, 0);
    init_color(2, 0, 1000, 0);
    endwin();
    system("clear");
}
