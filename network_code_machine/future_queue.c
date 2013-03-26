/*
 * future_queue.c
 *
 *  Created on: 2013-02-16
 *      Author: francis
 */

#include "future_queue.h"
#include <linux/kernel.h>
#include <linux/time.h>


/*
 * Initializes the future queue passed in
 */
void init_future_queue(ncm_future_queue_t* queue) {
	queue->first = 0;
	queue->length = 0;
}

/*
 * Sorts the future queue by expiry time so that the earliest time is first
 */
void sort_future_queue(ncm_future_queue_t* queue) {
	int i, j, jx, jxp;
	ncm_future_queue_el_t tmp;

	for(i = 0; i < (queue->length - 1); i++) {
		for(j = 0; j < (queue->length - i - 1); j++) {
			jx = ( queue->first + j ) % MAX_FUTURES;
			jxp = ( queue->first + j + 1 ) % MAX_FUTURES;
			if(queue->at[jx].expiry > queue->at[jxp].expiry) {
				tmp = queue->at[jxp];
				queue->at[jxp] = queue->at[jx];
				queue->at[jx] = tmp;
			}
		}
	}
}

/*
 * Checks if there are no elements in the queue
 */
bool is_empty(ncm_future_queue_t* queue) {
	return (bool) (queue->length == 0);
}

/*
 * Add a new future to the queue
 */
int push_future(ncm_future_queue_t* queue, u32 jmp, u64 wait_us) {
	int index;

	if(queue->length == MAX_FUTURES) {
		return FUTURE_Q_FULL;
	}

	index = (queue->first + queue->length) % MAX_FUTURES;
	queue->at[index].jmp_address = jmp;
	queue->at[index].wait_time = wait_us;
	queue->at[index].expiry = now_us() + wait_us;
	queue->length++;

	sort_future_queue(queue);

	return FUTURE_Q_OK;
}

/*
 * Removes the first element in the queue
 */
int pop_future(ncm_future_queue_t* queue, ncm_future_queue_el_t* out_el) {
	if(queue->length == 0) {
		return FUTURE_Q_EMPTY;
	}

	*out_el = queue->at[queue->first];
	queue->first = (queue->first + 1) % MAX_FUTURES;
	--queue->length;

	return FUTURE_Q_OK;
}

/*
 * Get's a pointer to the first element in the future queue
 */
ncm_future_queue_el_t* peek_future(ncm_future_queue_t* queue) {
	return &queue->at[queue->first];
}

