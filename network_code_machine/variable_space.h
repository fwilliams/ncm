/*
 * variable_space.h
 * The variable space for the Network Code interpreter
 *  Created on: 2013-03-05
 *      Author: Francis Williams
 */

#include <linux/types.h>
#include <linux/mutex.h>
#include <linux/mutex_rt.h>

#ifndef VARIABLE_SPACE_H_
#define VARIABLE_SPACE_H_

/*
 * Maximum number of variables that can exist in the variable space at one time
 */
#ifndef MAX_VARIABLES
#define MAX_VARIABLES 10
#endif

/*
 * Maximum number of bytes per variable
 */
#ifndef MAX_VAR_SIZE_BYTES
#define MAX_VAR_SIZE_BYTES 1500
#endif

/*
 * Struct representing a single variable
 *
 * NOTE:
 * The data is double buffered. This is done to prevent a possible priority inversion:
 * if a user space thread locks the mutex for writing and gets interrupted, this could block a
 * thread that needs access to the variable space (the NCM interpreter)
 */
struct variable {
	u32				id;								/* This variable's ID, 0 if unassigned */
	u32				length;							/* The number of bytes of data stored */
	u8				writeBuffer;					/* The current buffer being used */
	u8				data[2][MAX_VAR_SIZE_BYTES];	/* The (double buffered) data stored in this variable */
	struct mutex	var_mutex;						/* A mutex for this variable */
};

/*
 * Data structure containing the variables
 */
struct variable_space {
	struct variable	vars[MAX_VARIABLES];	/* Variables indexed by ID */
	u32				count;					/* The current number of variables allocated */
};

/* Initializes a new variable_space */
int init_variable_space(struct variable_space* varspace);

/* Create a new variable with the given data. This function returns the variable ID. */
u32 create_variable(struct variable_space* varspace);

/* De-allocates the variable for the variable ID argument */
int destroy_variable(struct variable_space* varspace, u32 varId);

/* Gets the data stored for the variable id argument */
int get_variable_data(struct variable_space* varspace, u32 varId, void* outData, u32* outLength);

/* Sets the data for the variable id argument to the data argument */
int set_variable_data(struct variable_space* varspace, u32 varId, void* data, u32 length);

#endif /* VARIABLE_SPACE_H_ */
