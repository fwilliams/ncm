/*
 * netcode_helper.h
 * Helper functions for the Network Code module
 *  Created on: 2013-02-16
 *      Author: Francis Williams
 */

#include <linux/types.h>

#ifndef NETCODE_HELPER_H_
#define NETCODE_HELPER_H_

/*
 * Prints to the kernel log if compiled in debug mode
 */
#ifdef __DEBUG__
#define debug_print(msg, args...) printk(KERN_INFO msg, ##args)
#else
#define debug_print(msg, args...) do { } while(0)
#endif

/*
 * Gets the number of milliseconds since the epoch
 */
u64 now_us(void);

/*
 * Returns 0 if even parity, 1 if odd parity
 */
u8 get_parity(u8 byte);

#endif /* NETCODE_HELPER_H_ */
