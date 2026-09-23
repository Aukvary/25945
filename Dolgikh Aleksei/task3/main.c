#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void print_uids(void) {
    printf("Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
}

void try_open(const char* filename) {
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("Failed to open file");
    } else {
        printf("File '%s' opened successfully\n", filename);
        fclose(fp);
    }
}

int main(void) {
    printf("--- Start process state ---\n");
    print_uids();
    try_open("data.txt");

    if (setuid(getuid()) != 0) {
        perror("setuid failed");
        return EXIT_FAILURE;
    }

    printf("\n--- State after setuid(getuid()) ---\n");
    print_uids();
    try_open("data.txt");

    return EXIT_SUCCESS;
}
