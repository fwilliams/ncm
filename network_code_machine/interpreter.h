/*
 * interp.h
 *
 *  Created on: 2013-02-16
 *      Author: francis
 */

#include <linux/types.h>
#include <linux/kthread.h>

#ifndef INTERP_H_
#define INTERP_H_

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

/*
 * Program error codes
 */
#define PROGRAM_OK 		0
#define PROGRAM_ERROR	1
#define INVALID_INSTR	2

/*
 * Instruction error codes
 */
#define INSTR_OK		0
#define INSTR_ERROR		1

/**
 * A single Network Code instruction
 */
struct instr_t {
	u8 	type;		/* The instruction type */
	u32 args[3];	/* The instruction args. Note some may be unused */
};

/*
 * The current state of a network code interpreter
 */
struct interpreter_t {
	u8 					program_counter;	/* Index to the current instruction */
	u8 					program_length;		/* The number of instructions in the program */
	struct instr_t* 	program;			/* Pointer to the netcode program */
	struct task_struct*	thread;				/* Handle to the interpreters thread */
};

/*
 * Initializes the interpreter passed in with the given program.
 * This will create and start a thread to run the interpreter in
 * and set the program counter to 0.
 */
int start_interpreter(struct interpreter_t* interpreter, struct instr_t* prog, u8 prog_len);

/*
 * Stops the interpreter thread and cleans up the interpreter
 */
int stop_interpreter(struct interpreter_t* interpreter);


#endif /* INTERP_H_ */
