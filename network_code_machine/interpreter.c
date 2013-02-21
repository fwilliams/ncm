/*
 * interpreter.c
 * The Network Code interpreter
 *
 *  Created on: 2013-02-16
 *      Author: Francis Williams
 */

#include "interpreter.h"

/*****************************************************************************
 * Interpreter instruction handlers
 * These functions map 1:1 with Network code instructions
 *****************************************************************************/

int handle_nop(struct interpreter_t* interpreter) {
	printk("NOP instruction reached at PC = %d\n", interpreter->program_counter);
	return INSTR_OK;
}

int handle_future(struct interpreter_t* interpreter) {
	printk("FUTURE instruction reached at PC = %d with argument %d\n",
			interpreter->program_counter,
			interpreter->program[interpreter->program_counter].args[0]);

	/*
	 * Insert element into the future queue
	 * 2 Cases:
	 * (i)	The future queue is not empty
	 * 			- Return
	 * (ii)	The future queue is empty
	 * 			- Start interpreter's timer
	 */
	return INSTR_OK;
}

int handle_halt(struct interpreter_t* interpreter) {
	printk("HALT instruction reached at PC = %d\n", interpreter->program_counter);
	return INSTR_OK;
}

int handle_end_of_program(struct interpreter_t* interpreter) {
	printk("END_OF_PROGRAM instruction reached at PC = %d\n",
			interpreter->program_counter);
	return INSTR_OK;
}

/*****************************************************************************
 * End interpreter instruction handlers
 * These functions map 1:1 with Network code instructions
 *****************************************************************************/


/*
 * The interpreter main thread function
 */
int interpreter_threadfn(void* data) {
	struct interpreter_t* interpreter = (struct interpreter_t*) data;

	printk("The interpreter is running! \n");
	printk("The program has %d instructions\n", interpreter->program_length);

	while(1) {
		switch( interpreter->program[interpreter->program_counter].type ) {
		case NOP:
			handle_nop(interpreter);
			break;
		case FUTURE:
			handle_future(interpreter);
			break;
		case HALT:
			handle_halt(interpreter);
			break;
		case END_OF_PROGRAM:
			return PROGRAM_OK;
		default:
			return INVALID_INSTR;
		}
		++interpreter->program_counter;
	}
	return PROGRAM_OK;
}

/*
 * Initializes the interpreter passed in with the given program.
 * This will create and start a thread to run the interpreter in
 * and set the program counter to 0.
 */
int start_interpreter(struct interpreter_t* interpreter, struct instr_t* prog, u8 prog_len) {
	interpreter->program_counter = 0;
	interpreter->program_length = prog_len;
	interpreter->program = prog;
	init_future_queue(&interpreter->future_queue);
	hrtimer_init(&interpreter->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	interpreter->thread = kthread_run(interpreter_threadfn, (void*) interpreter, "NCM interpreter");
	return 0;
}

/*
 * Stops the interpreter thread and cleans up the interpreter
 */
int stop_interpreter(struct interpreter_t* interpreter) {
	kthread_stop(interpreter->thread);
	return 0;
}




