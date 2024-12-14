#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
int main() {
   printf("Page fault count: %d\n", pagefault_count());
}
