/*
 * netcode_helper.h
 * Helper functions for the Network Code module
 *  Created on: 2013-02-16
 *      Author: Francis Williams
 */

#include <linux/types.h>

#ifndef NETCODE_HELPER_H_
#define NETCODE_HELPER_H_

#ifdef __DEBUG__
#define debug_print(msg, args...) printk(msg, ##args)
#else
#define debug_print(msg, args...) do { } while(0)
#endif

/*
 * Gets the number of milliseconds since the epoch
 */
u64 now_us(void);

#endif /* NETCODE_HELPER_H_ */
