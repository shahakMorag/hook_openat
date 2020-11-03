#include "syscall_hook.h"

#include <linux/string.h>
#include <linux/uaccess.h>

#define PATH_TO_REPLACE ("favourite_animal1")
#define ALTERNATIVE_PATH ("favourite_animal2")

SYSCALL_HOOK_DEFINE(openat, regs) {
    char filename[ARRAY_SIZE(PATH_TO_REPLACE) + 2] = { 0 };

    int copied = strncpy_from_user(filename, (void *)regs->si, ARRAY_SIZE(PATH_TO_REPLACE) + 1);
    if (0 < copied) {
        if (0 == strcmp(filename, PATH_TO_REPLACE)) {
            copy_to_user((void *)regs->si, ALTERNATIVE_PATH, strlen(ALTERNATIVE_PATH));
        }
    }

    return original_openat(regs);
}