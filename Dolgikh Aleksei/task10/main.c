#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[PARENT] Starting command: %s\n", argv[1]);
    fflush(stdout);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        execvp(argv[1], &argv[1]);
        perror("execvp failed");
        _exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid failed");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        printf(
            "[PARENT] Command exited normally with code: %d\n",
            WEXITSTATUS(status)
        );
    } else if (WIFSIGNALED(status)) {
        printf(
            "[PARENT] Command terminated by signal: %d (%s)\n",
            WTERMSIG(status), strsignal(WTERMSIG(status))
        );
    } else if (WIFSTOPPED(status)) {
        printf(
            "[PARENT] Command was stopped by signal: %d (%s)\n",
            WSTOPSIG(status), strsignal(WSTOPSIG(status))
        );
    }

    return EXIT_SUCCESS;
}
