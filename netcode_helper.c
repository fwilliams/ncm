/*
 * netcode_helper.c
 * Helper functions for the Network Code module
 *  Created on: 2013-02-16
 *      Author: Francis Williams
 */


#include "netcode_helper.h"

#include <linux/time.h>

static struct timeval helper_timeval;

/*
 * Gets the current timestamp in microseconds
 */
u64 now_us(void) {
	do_gettimeofday(&helper_timeval);
	return helper_timeval.tv_sec * USEC_PER_SEC + helper_timeval.tv_usec;
}