#include "syscall_hook.h"

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

int __init stevie_init(void) {
    return create_all_syscall_hooks();
}

void __exit stevie_exit(void) {
    remove_all_syscall_hooks();
}

MODULE_LICENSE("GPL");
module_init(stevie_init);
module_exit(stevie_exit);