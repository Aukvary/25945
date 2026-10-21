#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LINES 4096

int main(int argc, char* argv[]) {
    const char* filename = (argc > 1) ? argv[1] : "data.txt";

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        return EXIT_FAILURE;
    }

    long offsets[MAX_LINES];
    long lengths[MAX_LINES];
    int nlines = 0;

    long line_start = 0;
    long pos = 0;
    char c;

    while (read(fd, &c, 1) == 1) {
        if (c == '\n') {
            if (pos > line_start) {
                offsets[nlines] = line_start;
                lengths[nlines] = pos - line_start;
                nlines++;
            }
            line_start = pos + 1;
        }

        pos++;
    }

    if (line_start < pos) {
        offsets[nlines] = line_start;
        lengths[nlines] = pos - line_start;
        nlines++;
    }

    printf("--- Line table (offsets / lengths) ---\n");
    for (int i = 0; i < nlines; i++) {
        printf("%3d: off=%6ld len=%6ld\n", i + 1, offsets[i], lengths[i]);
    }
    printf("---------------------------------------\n");

    int num;
    while (1) {
        printf("Enter line number (0 to quit): ");
        fflush(stdout);
        if (scanf("%d", &num) != 1) {
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

        char buf[4096];
        long len = lengths[num - 1];
        if (lseek(fd, offsets[num - 1], SEEK_SET) == -1) {
            perror("lseek failed");
            break;
        }
        long total = 0;
        while (total < len) {
            ssize_t n = read(fd, buf + total, (size_t)(len - total));
            if (n <= 0) {
                if (n < 0) {
                    perror("read failed");
                }
                break;
            }
            total += n;
        }
        buf[total] = '\0';
        printf("%s\n", buf);
    }

    close(fd);
    return EXIT_SUCCESS;
}
