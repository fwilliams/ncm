/*
 * ncm_translator_test.cpp
 *
 *  Created on: 2013-04-06
 *      Author: Francis Williams
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE ncm_translator

#include <boost/test/unit_test.hpp>
#include <stdint.h>
#include "translator.h"

BOOST_AUTO_TEST_CASE(test_future_is_translated_correctly) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_FUTURE << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: delay value
	instr |= 237;

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, FUTURE);
	BOOST_CHECK_EQUAL(result.args[0], 42);
	BOOST_CHECK_EQUAL(result.args[1], 237);
}

BOOST_AUTO_TEST_CASE(test_halt_is_translated_correctly) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_HALT << 28);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, HALT);
}

BOOST_AUTO_TEST_CASE(test_if_always_false) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_ALWAYS_FALSE << 16);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], ALWAYS_FALSE);
	BOOST_CHECK_EQUAL(result.args[1], 42);
}

BOOST_AUTO_TEST_CASE(test_if_always_true) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_ALWAYS_TRUE << 16);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], ALWAYS_TRUE);
	BOOST_CHECK_EQUAL(result.args[1], 42);
}

BOOST_AUTO_TEST_CASE(test_if_chan_xy_rcv_buf_empty) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_BUF_EMPTY << 16);

	instr |= (199 << 8);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], CHANNEL_XY_RCV_BUF_EMTPY);
	BOOST_CHECK_EQUAL(result.args[1], 42);
	BOOST_CHECK_EQUAL(result.args[2], 199);
}

BOOST_AUTO_TEST_CASE(test_if_send_buf_empty) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_BUF_EMPTY << 16);

	instr |= 1;

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], SEND_BUFFER_EMPTY);
	BOOST_CHECK_EQUAL(result.args[1], 42);
}

BOOST_AUTO_TEST_CASE(test_if_test_count) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_TEST_COUNT << 16);

	instr |= 1234;

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], TEST_COUNT);
	BOOST_CHECK_EQUAL(result.args[1], 42);
	BOOST_CHECK_EQUAL(result.args[2], 1234);
}

BOOST_AUTO_TEST_CASE(test_if_status_test) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_STATUS_TEST << 16);

	instr |= 1234;

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], STATUS_TEST);
	BOOST_CHECK_EQUAL(result.args[1], 42);
	BOOST_CHECK_EQUAL(result.args[2], 1234);
}

BOOST_AUTO_TEST_CASE(test_if_equal_var_var) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_EQUAL_VAR_VAR << 16);

	instr |= (234 << 8);
	instr |= 125;

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], EQUAL_VAR_VAR);
	BOOST_CHECK_EQUAL(result.args[1], 42);
	BOOST_CHECK_EQUAL(result.args[2], 234);
	BOOST_CHECK_EQUAL(result.args[3], 125);
}

BOOST_AUTO_TEST_CASE(test_if_greater_var_var) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_GREATER_VAR_VAR << 16);

	instr |= (234 << 8);
	instr |= 125;

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], GREATER_VAR_VAR);
	BOOST_CHECK_EQUAL(result.args[1], 42);
	BOOST_CHECK_EQUAL(result.args[2], 234);
	BOOST_CHECK_EQUAL(result.args[3], 125);
}

BOOST_AUTO_TEST_CASE(test_if_less_var_var) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_LESS_VAR_VAR << 16);

	instr |= (234 << 8);
	instr |= 125;

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], LESS_VAR_VAR);
	BOOST_CHECK_EQUAL(result.args[1], 42);
	BOOST_CHECK_EQUAL(result.args[2], 234);
	BOOST_CHECK_EQUAL(result.args[3], 125);
}

BOOST_AUTO_TEST_CASE(test_if_test_var_not_zero) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_TEST_VAR << 16);

	instr |= (234 << 8);
	instr |= (1 << 3);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], TEST_VAR);
	BOOST_CHECK_EQUAL(result.args[1], 42);
	BOOST_CHECK_EQUAL(result.args[2], 234);
	BOOST_CHECK_EQUAL(result.args[3], VAR_IS_NONZERO);
}

BOOST_AUTO_TEST_CASE(test_if_test_var_is_zero) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_TEST_VAR << 16);

	instr |= (234 << 8);
	instr |= (1 << 2);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], TEST_VAR);
	BOOST_CHECK_EQUAL(result.args[1], 42);
	BOOST_CHECK_EQUAL(result.args[2], 234);
	BOOST_CHECK_EQUAL(result.args[3], VAR_IS_ZERO);
}

BOOST_AUTO_TEST_CASE(test_if_test_var_has_even_parity) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_TEST_VAR << 16);

	instr |= (234 << 8);
	instr |= (1 << 1);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], TEST_VAR);
	BOOST_CHECK_EQUAL(result.args[1], 42);
	BOOST_CHECK_EQUAL(result.args[2], 234);
	BOOST_CHECK_EQUAL(result.args[3], VAR_EVEN_PARITY);
}

BOOST_AUTO_TEST_CASE(test_if_test_var_has_odd_parity) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_TEST_VAR << 16);

	instr |= (234 << 8);
	instr |= 1;

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], TEST_VAR);
	BOOST_CHECK_EQUAL(result.args[1], 42);
	BOOST_CHECK_EQUAL(result.args[2], 234);
	BOOST_CHECK_EQUAL(result.args[3], VAR_ODD_PARITY);
}

BOOST_AUTO_TEST_CASE(test_if_test_var_priority_of_tests) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_TEST_VAR << 16);

	instr |= (234 << 8);
	instr |= 1;
	instr |= (1 << 3);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], TEST_VAR);
	BOOST_CHECK_EQUAL(result.args[1], 42);
	BOOST_CHECK_EQUAL(result.args[2], 234);
	BOOST_CHECK_EQUAL(result.args[3], VAR_IS_NONZERO);
}

BOOST_AUTO_TEST_CASE(test_if_var_queue_empty) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_VAR_QUEUE_EMPTY << 16);

	instr |= (234 << 8);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], VAR_QUEUE_EMPTY);
	BOOST_CHECK_EQUAL(result.args[1], 42);
	BOOST_CHECK_EQUAL(result.args[2], 234);
}

BOOST_AUTO_TEST_CASE(test_if_var_queue_full) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_IF << 28);

	// Argument 1: jump address
	instr |= (42 << 20);

	// Argument 2: guard mask
	instr |= (HW_GRD_VAR_QUEUE_FULL << 16);

	instr |= (234 << 8);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, IF);
	BOOST_CHECK_EQUAL(result.args[0], VAR_QUEUE_FULL);
	BOOST_CHECK_EQUAL(result.args[1], 42);
	BOOST_CHECK_EQUAL(result.args[2], 234);
}

BOOST_AUTO_TEST_CASE(test_create) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_CREATE << 28);

	// Argument 1: The variable ID
	instr |= (134 << 8);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, CREATE);
	BOOST_CHECK_EQUAL(result.args[0], 134);
}

BOOST_AUTO_TEST_CASE(test_send) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_SEND << 28);

	// Argument 1: The channel ID
	instr |= (134 << 16);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, SEND);
	BOOST_CHECK_EQUAL(result.args[0], 134);
	BOOST_CHECK_EQUAL(result.args[1], 0);
}

BOOST_AUTO_TEST_CASE(test_receive) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_RECEIVE << 28);

	// Argument 1: Channel ID
	instr |= (134 << 16);

	// Argument 2: Variable ID
	instr |= (211 << 8);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, RECEIVE);
	BOOST_CHECK_EQUAL(result.args[0], 134);
	BOOST_CHECK_EQUAL(result.args[1], 211);
}

BOOST_AUTO_TEST_CASE(test_nop) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_NOP << 28);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, NOP);
}

BOOST_AUTO_TEST_CASE(test_sync_slave) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_SYNC << 28);

	// Argument 1: Mode
	instr |= (0 << 27);

	// Argument 2: timeout
	instr |= 1259;

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, SYNC);
	BOOST_CHECK_EQUAL(result.args[0], SYNC_SLAVE);
	BOOST_CHECK_EQUAL(result.args[1], 1259);
}

BOOST_AUTO_TEST_CASE(test_sync_master) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_SYNC << 28);

	// Argument 1: Mode
	instr |= (1 << 27);

	// Argument 2: timeout
	instr |= 219;

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, SYNC);
	BOOST_CHECK_EQUAL(result.args[0], SYNC_MASTER);
	BOOST_CHECK_EQUAL(result.args[1], 219);
}

BOOST_AUTO_TEST_CASE(test_count_do_nothing) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_COUNT << 28);

	// Argument 1: Mode
	instr |= (0 << 24);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, NOP);
}

BOOST_AUTO_TEST_CASE(test_count_clear) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_COUNT << 28);

	// Argument 1: Mode
	instr |= (1 << 24);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, CLEAR_COUNTER);
}

BOOST_AUTO_TEST_CASE(test_count_increment) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_COUNT << 28);

	// Argument 1: Mode
	instr |= (2 << 24);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, ADD_TO_COUNTER);
	BOOST_CHECK_EQUAL(result.args[0], 0);
	BOOST_CHECK_EQUAL(result.args[1], 1);
}

BOOST_AUTO_TEST_CASE(test_count_decrement) {
	uint32_t instr = 0;
	ncm_instr_t result;

	// Instruction type
	instr |= (HW_COUNT << 28);

	// Argument 1: Mode
	instr |= (3 << 24);

	result = get_instruction(instr);

	BOOST_CHECK_EQUAL(result.type, SUB_FROM_COUNTER);
	BOOST_CHECK_EQUAL(result.args[0], 0);
	BOOST_CHECK_EQUAL(result.args[1], 1);
}
