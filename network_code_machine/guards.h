/*
 * guards.h
 *
 *  Created on: 2013-03-22
 *      Author: Francis Williams
 */

#include <linux/types.h>

#include "interpreter.h"

#ifndef GUARDS_H_
#define GUARDS_H_

#define ALWAYS_TRUE					0
#define ALWAYS_FALSE				1
#define CHANNEL_XY_RCV_BUF_EMTPY	2
#define SEND_BUFFER_EMPTY			3
#define TEST_COUNT					4
#define STATUS_TEST					5
#define EQUAL_VAR_VAR				6
#define GREATER_VAR_VAR				7
#define LESS_VAR_VAR				8
#define TEST_VAR					9
#define VAR_QUEUE_EMPTY				10
#define VAR_QUEUE_FULL				11

#define GUARD_OK					0
#define INVALID_GUARD				1

/*
 * Tests if a guard is true or false
 */
bool test_guard(ncm_interpreter_t* interpreter, u32 guard_id, u32* args, u32* status);

#endif /* GUARDS_H_ */
