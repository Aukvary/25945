#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LINES 4096
#define TIMEOUT 5

static char* g_map = NULL;
static size_t g_size = 0;

static void on_alarm(int sig) {
    if (g_map != NULL && g_size > 0) {
        size_t off = 0;
        while (off < g_size) {
            ssize_t n = write(STDOUT_FILENO, g_map + off, g_size - off);
            if (n <= 0) {
                break;
            }
            off += (size_t)n;
        }
    }
    _exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
    const char* filename = (argc > 1) ? argv[1] : "data.txt";

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        return EXIT_FAILURE;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat failed");
        return EXIT_FAILURE;
    }
    if (st.st_size == 0) {
        fprintf(stderr, "File is empty.\n");
        return EXIT_FAILURE;
    }

    g_map = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (g_map == MAP_FAILED) {
        perror("mmap failed");
        return EXIT_FAILURE;
    }
    g_size = (size_t)st.st_size;
    close(fd);

    long offsets[MAX_LINES];
    long lengths[MAX_LINES];
    int nlines = 0;

    long line_start = 0;
    for (long pos = 0; pos < (long)g_size; pos++) {
        if (g_map[pos] == '\n') {
            if (nlines < MAX_LINES) {
                offsets[nlines] = line_start;
                lengths[nlines] = pos - line_start;
                nlines++;
            }
            line_start = pos + 1;
        }
    }
    if (line_start < (long)g_size && nlines < MAX_LINES) {
        offsets[nlines] = line_start;
        lengths[nlines] = (long)g_size - line_start;
        nlines++;
    }

    printf("--- Line table (offsets / lengths) ---\n");
    for (int i = 0; i < nlines; i++) {
        printf("%3d: off=%6ld len=%6ld\n", i + 1, offsets[i], lengths[i]);
    }
    printf("---------------------------------------\n");

    struct sigaction sa;
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction failed");
        return EXIT_FAILURE;
    }

    int num;
    while (1) {
        alarm(TIMEOUT);

        printf(
            "Enter line number (0 to quit, %d seconds to answer): ", TIMEOUT
        );
        fflush(stdout);

        int res = scanf("%d", &num);
        alarm(0);

        if (res != 1) {
            break;
        }
        if (num == 0) {
            break;
        }
        if (num < 1 || num > nlines) {
            fprintf(
                stderr, "Error: line number out of range (1..%d)\n", nlines
            );
            continue;
        }
        fwrite(g_map + offsets[num - 1], 1, (size_t)lengths[num - 1], stdout);
        printf("\n");
    }

    munmap(g_map, g_size);
    return EXIT_SUCCESS;
}
