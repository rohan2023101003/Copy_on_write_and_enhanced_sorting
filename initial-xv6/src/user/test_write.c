#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#define NUM_PAGES 10
#define PAGE_SIZE 4096
void test_write_without_fork(){
    char *pages[NUM_PAGES];
    for (int i = 0; i < NUM_PAGES; i++) {
        pages[i] = (char *) malloc(PAGE_SIZE);
        if (pages[i] == 0) {
            exit(1);
        }
    }
    for (int i = 0; i < NUM_PAGES; i++) {
        pages[i][0]=0; // Read to trigger page fault if needed
    }
    for (int i = 0; i < NUM_PAGES; i++) {
      free(pages[i]);
    }
    return;
}
void test_write() {
    // Allocate memory for NUM_PAGES, all of which should be read-only (initially).
    char *pages[NUM_PAGES];
    for (int i = 0; i < NUM_PAGES; i++) {
        pages[i] = (char *) malloc(PAGE_SIZE);
        if (pages[i] == 0) {
            exit(1);
        }
    }

    // Now, after forking, the pages should be shared by parent and child.
    // We will print the address to verify.
    int pid = fork();
    if (pid < 0) {
        exit(1);
    }

    if (pid == 0) {
        // In child process, we attempt to read the pages
        for (int i = 0; i < NUM_PAGES; i++) {
            pages[i][0]=0; // Read to trigger page fault if needed
        }
        exit(0);
    } else {
        // In parent process, we wait for child process
        wait(0);
        // Ensure the pages were shared, hence no page faults for read.
    }
    for (int i = 0; i < NUM_PAGES; i++) {
      free(pages[i]);
    }
    return;
}
int main() {
    int initial_page_faults = pagefault_count();
    test_write();
    printf("write-Only Test: Page faults = %d\n", pagefault_count() - initial_page_faults);
    initial_page_faults = pagefault_count();
    test_write_without_fork();
    printf("write-Only Test without fork: Page faults = %d\n", pagefault_count() - initial_page_faults);
    exit(0);
}
