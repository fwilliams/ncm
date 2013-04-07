/*
 * interpreter.h
 * The Network Code interpreter
 *  Created on: 2013-02-16
 *      Author: Francis Williams
 */

#include <linux/types.h>
#include <linux/kthread.h>
#include <linux/delay.h>

#include "future_queue.h"
#include "variable_space.h"
#include "counter.h"
#include "nc_net.h"

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

/*
 * The bytecodes representing the various netcode instructions
 */
#define FUTURE 				0
#define HALT 				1
#define IF 					2
#define MODE 				3
#define CREATE 				4
#define DESTROY 			5
#define SEND				6
#define RECEIVE				7
#define SYNC				8
#define HANDLE				9
#define GOTO				10
#define WAIT				11
#define XSEND				12
#define BACKUP_SEND			13
#define BACKUP_RECEIVE		14
#define NOP					15
#define END_OF_PROGRAM		16
#define SET_COUNTER			17
#define ADD_TO_COUNTER		18
#define CLEAR_COUNTER		19
#define SUB_FROM_COUNTER	20
/*
 * Special parameters to netcode instructions
 */
#define SYNC_MASTER 0
#define SYNC_SLAVE 1

/*
 * Program error codes
 */
#define PROGRAM_OK 		0
#define PROGRAM_ERROR	1
#define INVALID_INSTR	2

#define ERR_BIT_SYNC_TIMEOUT	15
#define ERR_BIT_RCV_FAULT		10
/*
 * A single Network Code instruction
 */
typedef struct netcode_instr {
	u8 	type;		/* The instruction type */
	u32 args[4];	/* The instruction args. Note some may be unused */
} ncm_instr_t;

/*
 * A Network Code program
 */
typedef struct netcode_program {
	ncm_instr_t* 	instructions;
	u32					length;
} ncm_program_t;

/*
 * Runtime parameters for the interpreter
 */
typedef struct netcode_interpreter_params {
	ncm_net_params_t network;
} ncm_interp_params_t;

/*
 * The current state of a network code interpreter
 */
typedef struct interpreter {
	u32						program_counter;	/* Index to the current instruction */
	u32						program_length;		/* The number of instructions in the program */
	ncm_instr_t* 			program;			/* Pointer to the netcode program */
	ncm_future_queue_t		future_queue;		/* The future queue */
	ncm_varspace_t			variable_space;		/* The variable space */
	ncm_counter_array_t		counters;			/* Array of counters */
	ncm_network_t			network;			/* NCM network abstraction */
	struct task_struct*		thread;				/* Handle to the interpreters thread */
	u16						error_bits;			/* Bit string of errors */
} ncm_interpreter_t;

/*
 * Initializes the interpreter passed in.
 * This will create the thread the interpreter runs in
 * and set the program counter to 0.
 */
int init_interpreter(ncm_interpreter_t* interpreter);

/*
 * Starts the interpreter thread with the given program and parameters after resetting the interpreter state.
 */
int start_interpreter(ncm_interpreter_t* interpreter, ncm_program_t* program, ncm_interp_params_t* params);

/*
 * Stops the interpreter thread.
 */
int stop_interpreter(ncm_interpreter_t* interpreter);

/*
 * Checks if the interpreter is running
 */
bool is_running(ncm_interpreter_t* interpreter);

/*
 * Checks if an error bit has been set
 */
bool is_error_set(ncm_interpreter_t* interpreter, u16 error_mask);

/*
 * Sets the error bit given by bit.
 * E.g. if bit = 15, the 15th bit from the left will be set to 1
 */
void set_error(ncm_interpreter_t* interpreter, u8 bit);

#endif /* INTERPRETER_H_ */
