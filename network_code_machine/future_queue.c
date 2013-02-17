/*
 * future_queue.c
 *
 *  Created on: 2013-02-16
 *      Author: francis
 */

#include "future_queue.h"

/*
 * Initializes the future queue passed in
 */
void init_future_queue(struct future_queue_t* queue) {
	queue->first = 0;
	queue->length = 0;
}

/*
 * Add a new future to the queue
 */
int add_future(struct future_queue_t* queue, u32 jmp, u32 wait_ms) {
	int index;
	if(queue->length == MAX_FUTURES) {
		return FUTURE_Q_FULL;
	}

	index = (queue->first + queue->length) % MAX_FUTURES;
	queue->at[index].jmp_address = jmp;
	queue->at[index].wait_time = wait_ms;
	queue->at[index].expiry = now_ms() + wait_ms;
	queue->length++;

	return FUTURE_Q_OK;
}

/*
 * Removes the first element in the queue
 */
int get_future(struct future_queue_t* queue, struct future_queue_el_t* out_el) {
	if(queue->length == 0) {
		return FUTURE_Q_EMPTY;
	}

	*out_el = queue->at[queue->first];
	queue->first = (queue->first + 1) % MAX_FUTURES;
	--queue->length;

	return FUTURE_Q_OK;
}

void sort_future_queue(struct future_queue_t* queue) {

}


