#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char* line;
    struct Node* next;
} Node;

int main(void) {
    char buf[256];

    Node* head = NULL;
    Node* tail = NULL;

    printf("Enter strings (a line starting with '.' ends input):\n");

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        if (buf[0] == '.') {
            break;
        }

        size_t len = strlen(buf);

        if (len > 0 && buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
            len--;
        }

        Node* node = malloc(sizeof(Node));
        if (node == NULL) {
            perror("malloc node failed");
            return EXIT_FAILURE;
        }

        node->line = malloc(len + 1);
        if (node->line == NULL) {
            perror("malloc line failed");
            return EXIT_FAILURE;
        }

        memcpy(node->line, buf, len + 1);
        node->next = NULL;

        if (tail == NULL) {
            head = tail = node;
        } else {
            tail->next = node;
            tail = node;
        }
    }

    printf("\n--- List contents ---\n");
    // for (Node* n = head; n != NULL; n = n->next) {
    //     printf("%s\n", n->line);
    // }
    //
    printf("%s\n", head->line);
    printf("%s\n", tail->line);

    Node* n = head;
    while (n != NULL) {
        Node* tmp = n;
        n = n->next;
        free(tmp->line);
        free(tmp);
    }

    return EXIT_SUCCESS;
}
