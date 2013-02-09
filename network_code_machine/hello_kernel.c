#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/time.h>
#include <linux/types.h>

struct timeval t;

int init_module() {
	do_gettimeofday(&t);
	printk(KERN_INFO "Started at time %lu",
			(u64) t.tv_sec * 1000 + (u64) t.tv_usec);
	return 0;
}

void cleanup_module(void) {
	do_gettimeofday(&t);
	printk(KERN_INFO "End time at %lu",
			(u64) t.tv_sec * 1000 + (u64) t.tv_usec);
}
