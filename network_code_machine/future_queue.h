/*
 * future_queue.h
 * The future queue for the Network Code interpreter
 *  Created on: 2013-02-16
 *      Author: Francis Williams
 */

#include <linux/types.h>

#include "netcode_helper.h"

#ifndef FUTURE_QUEUE_H_
#define FUTURE_QUEUE_H_

/*
 * Future queue error codes
 */
#define FUTURE_Q_OK		0
#define FUTURE_Q_FULL	1
#define FUTURE_Q_EMPTY	2

/*
 * The maximum length of the future queue
 */
#ifndef MAX_FUTURES
#define MAX_FUTURES		10
#endif

/*
 * An element in the future queue
 */
typedef struct future_queue_el {
	u32		jmp_address;	/* The address that the interpreter will jump to when the future occurs */
	u32		wait_time;		/* The requested wait time for this future in milliseconds */
	u64		expiry;			/* The exact timestamp when this future should occur */
} ncm_future_queue_el_t;

/*
 * The future queue
 */
typedef struct future_queue {
	u8 						first;				/* The index of the first element in the queue */
	u8						length;				/* The index of the last element in the queue */
	ncm_future_queue_el_t 	at[MAX_FUTURES];	/* The actual queue of futures */
} ncm_future_queue_t;

/*
 * Initializes the future queue passed in
 */
void init_future_queue(ncm_future_queue_t* queue);

/*
 * Checks if there are no elements in the queue
 */
bool is_empty(ncm_future_queue_t* queue);

/*
 * Add a new future to the queue
 */
int push_future(ncm_future_queue_t* queue, u32 jmp, u64 wait_us);

/*
 * Removes the first element in the queue
 */
int pop_future(ncm_future_queue_t* queue, ncm_future_queue_el_t* out_el);

/*
 * Get's a pointer to the first element in the future queue
 */
ncm_future_queue_el_t* peek_future(ncm_future_queue_t* queue);

#endif /* FUTURE_QUEUE_H_ */
