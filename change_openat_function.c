#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/unistd.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>

#define PATH_TO_REPLACE ("favourite_animal1")
#define ALTERNATIVE_PATH ("favourite_animal2")

static inline void my_write_cr0(unsigned long cr0) {
	asm volatile("mov %0,%%cr0" : "+r"(cr0), "+m"(__force_order));
}

#define REMOVE_WRITE_PROTECTION() do { my_write_cr0(read_cr0() & (~0x10000)); } while(0)
#define ADD_WRITE_PROTECTION() do { my_write_cr0(read_cr0() | (0x10000)); } while(0)

unsigned long long * g_syscall_table = NULL;
asmlinkage long (*g_original_openat)(struct pt_regs * regs) = NULL;

int change_syscall(unsigned int syscall_number, unsigned long long replacement) {
	if (NULL == g_syscall_table) {
		printk(KERN_ERR "syscall table address is invalid\n");
		return 0;
	}

	REMOVE_WRITE_PROTECTION();
	g_syscall_table[syscall_number] = replacement;
	ADD_WRITE_PROTECTION();
	return 1;
}

asmlinkage long openat_hook(struct pt_regs * regs) {
    char filename[ARRAY_SIZE(PATH_TO_REPLACE) + 2] = { 0 };

    int copied = strncpy_from_user(filename, (void *)regs->si, ARRAY_SIZE(PATH_TO_REPLACE) + 1);
    if (0 < copied) {
        if (0 == strcmp(filename, PATH_TO_REPLACE)) {
            copy_to_user((void *)regs->si, ALTERNATIVE_PATH, strlen(ALTERNATIVE_PATH));
        }
    }

    return g_original_openat(regs);
}

int __init replacement_init(void) {
	g_syscall_table = (void *)kallsyms_lookup_name("sys_call_table");
	if (NULL == g_syscall_table) {
		printk(KERN_ERR "couldn't find syscall table\n");
		return -1;
	}

	g_original_openat = (void *)g_syscall_table[__NR_openat];
	if (NULL == g_original_openat) {
		printk(KERN_ERR "openat is null\n");
		return -1;
	}

	if (!change_syscall(__NR_openat, (unsigned long long)&openat_hook)) {
		printk(KERN_ERR "failed to change openat to hook function\n");
		return -1;
	}

	return 0;
}

void __exit replacement_exit(void) {
    if (!change_syscall(__NR_openat, (unsigned long long)g_original_openat)) {
		printk(KERN_ERR "failed to change openat to original function\n");
    }
}

MODULE_LICENSE("GPL");
module_init(replacement_init);
module_exit(replacement_exit);
