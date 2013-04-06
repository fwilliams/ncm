/*
 * counter.c
 *
 *  Created on: 2013-03-22
 *      Author: Francis Williams
 */

#include "counter.h"

/* Resets the counter at counter_id */
void reset_counter(ncm_counter_array_t* carray, u32 counter_id) {
	carray->at[counter_id] = 0;
}

/* Sets the value of the counter at counter_id to value */
void set_counter(ncm_counter_array_t* carray, u32 counter_id, u32 value) {
	carray->at[counter_id] = value;
}

/* Adds amount to the counter at counter_id */
void add_to_counter(ncm_counter_array_t* carray, u32 counter_id, u32 amount) {
	carray->at[counter_id] += amount;
}

/* Subtracts amount from the counter at counter_id */
void sub_from_counter(ncm_counter_array_t* carray, u32 counter_id, u32 amount) {
	carray->at[counter_id] -= amount;
}
