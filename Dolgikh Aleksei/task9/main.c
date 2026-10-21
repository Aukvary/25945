#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    const char* filename = (argc > 1) ? argv[1] : "data.txt";

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        printf("[CHILD] I am the child process. My PID: %d\n", (int)getpid());
        fflush(stdout);
        execlp("cat", "cat", filename, (char*)NULL);
        perror("execlp failed");
        _exit(EXIT_FAILURE);
    }

    printf("[PARENT] I am the parent. My PID: %d\n", (int)getpid());
    printf("[PARENT] I have created a child. Child PID: %d\n", (int)pid);
    printf("[PARENT] Printing some text while the child is working...\n");

    int status;
    if (waitpid(pid, &status, 0) < 0) {
        perror("waitpid failed");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        printf(
            "[PARENT] Child exited normally with status: %d\n",
            WEXITSTATUS(status)
        );
    } else {
        printf("[PARENT] Child terminated abnormally\n");
    }

    printf(
        "[PARENT] Child has finished. This is the last line printed by "
        "parent.\n"
    );
    return EXIT_SUCCESS;
}
