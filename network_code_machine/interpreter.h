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
#define FUTURE 			0
#define HALT 			1
#define IF 				2
#define MODE 			3
#define CREATE 			4
#define DESTROY 		5
#define SEND			6
#define RECIEVE			7
#define HANDLE			8
#define GOTO			9
#define WAIT			10
#define XSEND			11
#define BACKUP_SEND		12
#define BACKUP_RECEIVE	13
#define NOP				14
#define END_OF_PROGRAM	15
#define SET_COUNTER		16
#define ADD_TO_COUNTER	17
#define CLEAR_COUNTER	18

/*
 * Program error codes
 */
#define PROGRAM_OK 		0
#define PROGRAM_ERROR	1
#define INVALID_INSTR	2

/*
 * A single Network Code instruction
 */
typedef struct netcode_instr {
	u8 	type;		/* The instruction type */
	u32 args[4];	/* The instruction args. Note some may be unused */
} netcode_instr_t;

/*
 * A Network Code program
 */
typedef struct netcode_program {
	netcode_instr_t* 	instructions;
	u32					length;
} netcode_program_t;

/*
 * Runtime parameters for the interpreter
 */
typedef struct netcode_interpreter_params {
	ncm_net_params_t network;
} interp_params_t;

/*
 * The current state of a network code interpreter
 */
typedef struct interpreter {
	u32						program_counter;	/* Index to the current instruction */
	u32						program_length;		/* The number of instructions in the program */
	netcode_instr_t* 		program;			/* Pointer to the netcode program */
	struct task_struct*		thread;				/* Handle to the interpreters thread */
	ncm_future_queue_t		future_queue;		/* The future queue */
	varspace_t				variable_space;		/* The variable space */
	counter_array_t			counters;			/* Array of counters */
	ncm_network_t			network;			/* NCM network abstraction */
} ncm_interpreter_t;

/*
 * Initializes the interpreter passed in with the given program.
 * This will create and start a thread to run the interpreter in
 * and set the program counter to 0.
 */
int start_interpreter(ncm_interpreter_t* interpreter, netcode_program_t* program, interp_params_t* params);

/*
 * Stops the interpreter thread and cleans up the interpreter
 */
int stop_interpreter(ncm_interpreter_t* interpreter);


#endif /* INTERPRETER_H_ */
