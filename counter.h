/*
 * counter.h
 *
 *  Created on: 2013-03-22
 *      Author: Francis Williams
 */

#include <linux/types.h>

#ifndef COUNTER_H_
#define COUNTER_H_

#ifndef MAX_COUNTERS
#define MAX_COUNTERS 10
#endif

/*
 * An array of counters (unsigned integers)
 */
typedef struct counters {
	u32	at[MAX_COUNTERS];
} counter_array_t;

/* Resets the counter at counter_id */
void reset_counter(counter_array_t* carray, u32 counter_id);

/* Sets the value of the counter at counter_id to value */
void set_counter(counter_array_t* carray, u32 counter_id, u32 value);

/* Adds amount to the counter at counter_id */
void add_to_counter(counter_array_t* carray, u32 counter_id, u32 amount);

#endif /* COUNTER_H_ */
