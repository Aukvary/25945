#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define COLS 40
#define BUF_SIZE 4096
#define MAX_VIS (BUF_SIZE + 2)
#define PROMPT "> "
#define PROMPT_COLS 2
#define CTRL_D 4
#define CTRL_W 23

static struct termios orig_termios;
static int raw_enabled = 0;

static char buffer[BUF_SIZE];
static size_t pos = 0;

static struct {
    size_t start;
    int len;
} vis[MAX_VIS];
static int nvis = 0;
static int cur_lines = 0;

static void disable_raw_mode() {
    if (raw_enabled) {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
        raw_enabled = 0;
    }
}

static void enable_raw_mode() {
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }

    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }

    raw_enabled = 1;
    atexit(disable_raw_mode);
}

static void layout() {
    nvis = 1;
    vis[0].start = 0;
    vis[0].len = 0;

    int col = PROMPT_COLS;
    size_t i = 0;

    while (i < pos) {
        if (buffer[i] == ' ') {
            if (col < COLS) {
                col++;
                vis[nvis - 1].len++;
            }
            i++;
            continue;
        }

        size_t j = i;
        while (j < pos && buffer[j] != ' ')
            j++;
        size_t wlen = j - i;

        if (col + (int)wlen > COLS) {
            nvis++;
            vis[nvis - 1].start = i;
            vis[nvis - 1].len = 0;
            col = 0;
        }

        while (wlen > (size_t)COLS) {
            vis[nvis - 1].len += COLS;
            i += COLS;
            wlen -= COLS;
            nvis++;
            vis[nvis - 1].start = i;
            vis[nvis - 1].len = 0;
            col = 0;
        }

        vis[nvis - 1].len += (int)wlen;
        col += (int)wlen;
        i = j;
    }
}

static void redraw() {
    layout();

    int total = nvis;
    if (cur_lines > total)
        total = cur_lines;

    if (cur_lines > 1)
        printf("\033[%dA", cur_lines - 1);

    for (int v = 0; v < total; v++) {
        if (v == 0)
            printf("\r\033[K%s", PROMPT);
        else
            printf("\n\r\033[K");
        if (v < nvis)
            printf("%.*s", vis[v].len, buffer + vis[v].start);
    }

    cur_lines = nvis;
    fflush(stdout);
}

static void bell() {
    printf("\a");
    fflush(stdout);
}

int main() {
    enable_raw_mode();

    cc_t erase = orig_termios.c_cc[VERASE];
    cc_t kill = orig_termios.c_cc[VKILL];

    redraw();

    for (;;) {
        unsigned char c;

        if (read(STDIN_FILENO, &c, 1) != 1)
            break;

        if (c == CTRL_D) {
            if (pos == 0)
                break;
            bell();
            continue;
        }

        if (c == '\n' || c == '\r') {
            pos = 0;
            cur_lines = 1;
            printf("\n");
            fflush(stdout);
            redraw();
            continue;
        }

        if (c == erase) {
            if (pos > 0)
                pos--;
            redraw();
            continue;
        }

        if (c == kill) {
            pos = 0;
            redraw();
            continue;
        }

        if (c == CTRL_W) {
            size_t i = pos;
            while (i > 0 && buffer[i - 1] == ' ')
                i--;
            while (i > 0 && buffer[i - 1] != ' ')
                i--;
            while (i > 0 && buffer[i - 1] == ' ')
                i--;
            pos = i;
            redraw();
            continue;
        }

        if (isprint(c)) {
            if (pos < BUF_SIZE - 1) {
                buffer[pos++] = (char)c;
                redraw();
            } else {
                bell();
            }
            continue;
        }

        bell();
    }

    printf("\nExiting...\n");
    disable_raw_mode();
    return EXIT_SUCCESS;
}
