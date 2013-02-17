/*
 * netcode_helper.c
 * Helper functions for the Network Code module
 *  Created on: 2013-02-16
 *      Author: Francis Williams
 */


#include "netcode_helper.h"

#include <linux/time.h>

static struct timeval helper_timeval;

unsigned long now_ms(void) {
	do_gettimeofday(&helper_timeval);
	return helper_timeval.tv_sec * 1000 + helper_timeval.tv_usec;
}
