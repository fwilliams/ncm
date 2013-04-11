/*
 * translator.h
 *
 *  Created on: 2013-03-30
 *      Author: francis
 */

#include <stdint.h>

#ifndef TRANSLATOR_H_
#define TRANSLATOR_H_

/*
 * The bytecodes representing the various netcode instructions
 */
#define FUTURE 						0
#define HALT 						1
#define IF 							2
#define MODE 						3
#define CREATE 						4
#define DESTROY 					5
#define SEND						6
#define RECEIVE						7
#define SYNC						8
#define HANDLE						9
#define GOTO						10
#define WAIT						11
#define XSEND						12
#define BACKUP_SEND					13
#define BACKUP_RECEIVE				14
#define NOP							15
#define END_OF_PROGRAM				16
#define SET_COUNTER					17
#define ADD_TO_COUNTER				18
#define CLEAR_COUNTER				19
#define SUB_FROM_COUNTER			20

/*
 * Parameters for sync instruction
 */
#define SYNC_MASTER					0
#define SYNC_SLAVE					1

/*
 * The bytecodes representing different types of guards
 */
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

/*
 * Test flags for test_variable()
 */
#define VAR_IS_ZERO		0
#define VAR_IS_NONZERO	1
#define VAR_EVEN_PARITY	2
#define VAR_ODD_PARITY	3

/*
 * Hardware implementation bytecodes
 */
#define HW_FUTURE					5
#define HW_HALT						4
#define HW_IF						7
#define HW_CREATE					1
#define HW_SEND						2
#define HW_RECEIVE					3
#define HW_NOP						0
#define HW_SYNC						15
#define HW_COUNT					8

#define HW_GRD_ALWAYS_TRUE			7
#define HW_GRD_ALWAYS_FALSE			0
#define HW_GRD_BUF_EMPTY			1
#define HW_GRD_TEST_COUNT			2
#define HW_GRD_STATUS_TEST			3
#define HW_GRD_EQUAL_VAR_VAR		8
#define HW_GRD_GREATER_VAR_VAR		9
#define HW_GRD_LESS_VAR_VAR			10
#define HW_GRD_TEST_VAR				11
#define HW_GRD_VAR_QUEUE_EMPTY		5
#define HW_GRD_VAR_QUEUE_FULL		6

/*
 * A single Network Code instruction
 */
typedef struct netcode_instr {
	uint8_t 	type;		/* The instruction type */
	uint32_t 	args[4];	/* The instruction args. Note some may be unused */
} ncm_instr_t;

/*
 * A Network Code program
 */
typedef struct netcode_program {
	ncm_instr_t* 	instructions;
	uint32_t		length;
} ncm_program_t;


#define TRANSLATION_OK 0

ncm_instr_t get_instruction(uint32_t bytecode);


#endif /* TRANSLATOR_H_ */
