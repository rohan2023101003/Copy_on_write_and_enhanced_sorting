#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define NUM_PAGES 10
#define PAGE_SIZE 4096

void test_read_without_fork() {
    char *pages[NUM_PAGES];
    for (int i = 0; i < NUM_PAGES; i++) {
        pages[i] = (char *) malloc(PAGE_SIZE);
        if (pages[i] == 0) {
            printf("malloc failed\n");
            exit(1);
        }
    }

    for (int i = 0; i < NUM_PAGES; i++) {
        char val = pages[i][0];
        if (val == 0) {
            // do nothing
        }
    }

    for (int i = 0; i < NUM_PAGES; i++) {
        free(pages[i]);
    }
    return;
}

void test_read() {
    char *pages[NUM_PAGES];
    for (int i = 0; i < NUM_PAGES; i++) {
        pages[i] = (char *) malloc(PAGE_SIZE);
        if (pages[i] == 0) {
            exit(1);
        }
    }

    int pid = fork();
    if (pid < 0) {
        exit(1);
    }

    if (pid == 0) {
        for (int i = 0; i < NUM_PAGES; i++) {
            char val = pages[i][0];
            if (val == 0) {
                // do nothing
            }
        }
        exit(0);
    } else {
        wait(0);
    }

    for (int i = 0; i < NUM_PAGES; i++) {
        free(pages[i]);
    }
    return;
}

int main() {
    int initial_page_faults = pagefault_count();
    test_read();
    printf("Read-Only Test with fork: Page faults = %d\n", pagefault_count() - initial_page_faults);

    initial_page_faults = pagefault_count();
    test_read_without_fork();
    printf("Read-Only Test without fork: Page faults = %d\n", pagefault_count() - initial_page_faults);

    exit(0);
}
