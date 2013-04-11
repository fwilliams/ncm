/*
 * translator_main.c
 *
 *  Created on: 2013-04-06
 *      Author: Francis Williams
 */

#include <stdio.h>
#include <stdlib.h>

#include "translator.h"

inline void swap_endian(uint32_t &val) {
	val = (val<<24) | ((val<<8) & 0x00ff0000) |
		  ((val>>8) & 0x0000ff00) | (val>>24);
}

int main(int argc, char** argv) {
	uint32_t in_bytecode;
	ncm_instr_t out_instr;

	while(true) {
		// Read in a bytecode
		if( fread(&in_bytecode, sizeof(uint32_t), 1, stdin) < 1 ) {
			break; // We've reached the end of the input. Stop.
		}

		swap_endian(in_bytecode);

		// Translate the bytecode
		out_instr = get_instruction(in_bytecode);

		// Write the translated instruction out
		fwrite(&out_instr, sizeof(ncm_instr_t), 1, stdout);
	}

	return 0;
}




