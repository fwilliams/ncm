/*
 * netcode_helper.c
 * Helper functions for the Network Code module
 *  Created on: 2013-02-16
 *      Author: Francis Williams
 */


#include "netcode_helper.h"

#include <linux/time.h>

/*
 * Gets the current timestamp in microseconds
 */
u64 now_us(void) {
	struct timeval helper_timeval;
	do_gettimeofday(&helper_timeval);
	return helper_timeval.tv_sec * USEC_PER_SEC + helper_timeval.tv_usec;
}

/*
 * Returns 0 if even parity, 1 if odd parity
 */
u8 get_parity(u8 byte) {
	u8 i;
	u8 acc;

	acc = 0;

	for(i = 0; i < 8; i++) {
		acc += (byte >> i) & 1;
	}

	return acc & 1;
}
