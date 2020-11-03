#ifndef __SYSCALL_HOOK__
#define __SYSCALL_HOOK__

#include <linux/printk.h>
#include <linux/unistd.h>

typedef enum {
    SUCCESS = 0,
    FAILURE
} result_t;

typedef asmlinkage long (*syscal_ptr_t)(struct pt_regs * regs);

typedef struct {
    syscal_ptr_t original;
    syscal_ptr_t hook;
} syscall_hook_t;

syscall_hook_t syscalls_hooks[__NR_syscalls] = { 0 };
syscal_ptr_t * g_syscall_table = NULL;

#define SYSCALL_HOOK_DEFINE(syscall_name, regs) \
asmlinkage long syscall_name##_hook(struct pt_regs * regs); \
syscalls_hooks[__NR_##syscall_name].hook = &syscall_name##_hook; \
asmlinkage long original_##syscall_name(struct pt_regs * regs) \
{ \
    return syscalls_hooks[__NR_##syscall_name].original(regs); \
} \
asmlinkage long syscall_name##_hook(struct pt_regs * regs)

result_t change_syscall(unsigned int syscall_number, syscal_ptr_t replacement);
result_t create_all_syscall_hooks(void);
result_t remove_all_syscall_hooks(void);

#endif // !__SYSCALL_HOOK__