/*
 * translator.c
 *	Mmmmmm... spaghetti.
 *  Created on: 2013-03-30
 *      Author: francis
 *
 */

#include "translator.h"
#include <stdio.h>

#define MASK4					(uint32_t) 0x0000F
#define MASK8					(uint32_t) 0x000FF
#define MASK16					(uint32_t) 0x0FFFF
#define MASK20					(uint32_t) 0xFFFFF

uint8_t get_instr_type(uint32_t bytecode) {
	return (uint8_t) (bytecode >> 28);
}

ncm_instr_t pack_future(uint32_t bytecode) {
	ncm_instr_t instr;

	instr.type = FUTURE;

	instr.args[0] = ((bytecode >> 20) & MASK8);
	instr.args[1] = (bytecode & MASK16);

	return instr;
}

ncm_instr_t pack_halt(uint32_t bytecode) {
	ncm_instr_t instr;

	instr.type = HALT;

	return instr;
}

void pack_guard_args(uint32_t guard_mask, uint32_t* out_args) {
	uint32_t type = (guard_mask >> 16);

	switch(type) {
	case HW_GRD_ALWAYS_TRUE:
		out_args[0] = ALWAYS_TRUE;
		break;
	case HW_GRD_ALWAYS_FALSE:
		out_args[0] = ALWAYS_FALSE;
		break;
	case HW_GRD_BUF_EMPTY:
		if( (guard_mask & 1) ) {						// SendBufferEmpy
			out_args[0] = SEND_BUFFER_EMPTY;
		} else { 										// ChannelXYReceiveBufferEmpty
			out_args[0] = CHANNEL_XY_RCV_BUF_EMTPY;
			out_args[2] = ((guard_mask >> 8) & MASK8);
		}
		break;
	case HW_GRD_TEST_COUNT:
		out_args[0] = TEST_COUNT;
		out_args[2] = (guard_mask & MASK16);
		break;
	case HW_GRD_STATUS_TEST:
		out_args[0] = STATUS_TEST;
		out_args[2] = (guard_mask & MASK16);
		break;
	case HW_GRD_EQUAL_VAR_VAR:
		out_args[0] = EQUAL_VAR_VAR;
		out_args[2] = ((guard_mask >> 8) & MASK8);
		out_args[3] = (guard_mask & MASK8);
		break;
	case HW_GRD_GREATER_VAR_VAR:
		out_args[0] = GREATER_VAR_VAR;
		out_args[2] = ((guard_mask >> 8) & MASK8);
		out_args[3] = (guard_mask & MASK8);
		break;
	case HW_GRD_LESS_VAR_VAR:
		out_args[0] = LESS_VAR_VAR;
		out_args[2] = ((guard_mask >> 8) & MASK8);
		out_args[3] = (guard_mask & MASK8);
		break;
	case HW_GRD_TEST_VAR:
		out_args[0] = TEST_VAR;
		out_args[2] = ((guard_mask >> 8) & MASK8);
		if(guard_mask & 8) {
			out_args[3] = VAR_IS_NONZERO;
		} else if(guard_mask & 4) {
			out_args[3] = VAR_IS_ZERO;
		} else if(guard_mask & 2) {
			out_args[3] = VAR_EVEN_PARITY;
		} else if(guard_mask & 1) {
			out_args[3] = VAR_ODD_PARITY;
		}
		break;
	case HW_GRD_VAR_QUEUE_EMPTY:
		out_args[0] = VAR_QUEUE_EMPTY;
		out_args[2] = ((guard_mask >> 8) & MASK8);
		break;
	case HW_GRD_VAR_QUEUE_FULL:
		out_args[0] = VAR_QUEUE_FULL;
		out_args[2] = ((guard_mask >> 8) & MASK8);
		break;
	}
}

ncm_instr_t pack_if(uint32_t bytecode) {
	ncm_instr_t instr;
	uint32_t guard_mask;

	instr.type = IF;
	instr.args[1] = ((bytecode >> 20) & MASK8);

	guard_mask = (bytecode & MASK20);

	pack_guard_args(guard_mask, &instr.args[0]);

	return instr;
}

ncm_instr_t pack_create(uint32_t bytecode) {
	ncm_instr_t instr;

	instr.type = CREATE;
	instr.args[0] = ((bytecode >> 8) & MASK8);

	return instr;
}

ncm_instr_t pack_send(uint32_t bytecode) {
	ncm_instr_t instr;

	instr.type = SEND;
	instr.args[0] = ((bytecode >> 16) & MASK8);
	instr.args[1] = 0; // Hard-coded Message ID since the hardware only has one message buffer

	return instr;
}

ncm_instr_t pack_receive(uint32_t bytecode) {
	ncm_instr_t instr;

	instr.type = RECEIVE;
	instr.args[0] = ((bytecode >> 16) & MASK8);
	instr.args[1] = ((bytecode >> 8) & MASK8);

	return instr;
}

ncm_instr_t pack_nop(uint32_t bytecode) {
	ncm_instr_t instr;
	instr.type = NOP;

	return instr;
}

ncm_instr_t pack_sync(uint32_t bytecode) {
	ncm_instr_t instr;

	instr.type = SYNC;

	if((bytecode >> 27) & 1) {
		instr.args[0] = SYNC_MASTER;
		instr.args[1] = (bytecode & MASK8);
	} else {
		instr.args[0] = SYNC_SLAVE;
		instr.args[1] = (bytecode & MASK16);
	}

	return instr;
}

ncm_instr_t pack_count(uint32_t bytecode) {
	ncm_instr_t instr;

	if(((bytecode >> 24) & 3) == 0) {
		instr.type = NOP;
	} else if(((bytecode >> 24) & 3) == 1) {
		instr.type = CLEAR_COUNTER;
	} else if(((bytecode >> 24) & 3) == 2) {
		instr.type = ADD_TO_COUNTER;
		instr.args[0] = 0;
		instr.args[1] = 1;
	} else if(((bytecode >> 24) & 3) == 3) {
		instr.type = SUB_FROM_COUNTER;
		instr.args[0] = 0;
		instr.args[1] = 1;
	}
	return instr;
}

ncm_instr_t get_instruction(uint32_t bytecode) {
	uint8_t type = get_instr_type(bytecode);

	switch(type) {
	case HW_FUTURE:
		return pack_future(bytecode);
	case HW_HALT:
		return pack_halt(bytecode);
	case HW_IF:
		return pack_if(bytecode);
	case HW_CREATE:
		return pack_create(bytecode);
	case HW_SEND:
		return pack_send(bytecode);
	case HW_RECEIVE:
		return pack_receive(bytecode);
	case HW_NOP:
		return pack_nop(bytecode);
	case HW_SYNC:
		return pack_sync(bytecode);
	case HW_COUNT:
		return pack_count(bytecode);
	}
}


