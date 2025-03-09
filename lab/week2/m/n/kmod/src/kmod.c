// kmod/src/kmod.c (简单内核模块)
#include <linux/module.h>
#include <linux/kernel.h>

int init_module(void) {
    printk(KERN_INFO "droomkmod loaded\n");
    return 0;
}

void cleanup_module(void) {
    printk(KERN_INFO "droomkmod unloaded\n");
}

MODULE_LICENSE("GPL");
