#include "syscall_hook.h"

#include <linux/kallsyms.h>

static inline void my_write_cr0(unsigned long cr0) {
	asm volatile("mov %0,%%cr0" : "+r"(cr0), "+m"(__force_order));
}

#define REMOVE_WRITE_PROTECTION() do { my_write_cr0(read_cr0() & (~0x10000)); } while(0)
#define ADD_WRITE_PROTECTION() do { my_write_cr0(read_cr0() | (0x10000)); } while(0)

result_t change_syscall(unsigned int syscall_number, syscal_ptr_t replacement) {
	if (NULL == g_syscall_table) {
		printk(KERN_ERR "syscall table address is invalid\n");
		return FAILURE;
	}

	REMOVE_WRITE_PROTECTION();
	g_syscall_table[syscall_number] = replacement;
	ADD_WRITE_PROTECTION();
	return SUCCESS;
}

result_t create_all_syscall_hooks(void) {
    g_syscall_table = (void *)kallsyms_lookup_name("sys_call_table");
	if (NULL == g_syscall_table) {
		printk(KERN_ERR "couldn't find syscall table\n");
		return -1;
	}

    for (int i = 0; i < __NR_syscalls; ++i) {
        if (NULL == syscalls_hooks[i].hook) {
            continue;
        }

        syscalls_hooks[i].original = g_syscall_table[i];
        (void)change_syscall(i, syscalls_hooks[i].hook);
    }
}

result_t remove_all_syscall_hooks(void) {
    if (NULL == g_syscall_table) {
        printk(KERN_ERR "syscall table address is invalid\n");
        return FAILURE;
    }

    for (int i = 0; i < __NR_syscalls; ++i) {
        if (NULL == syscalls_hooks[i].hook) {
            continue;
        }

        (void)change_syscall(i, syscalls_hooks[i].original);
    }
}