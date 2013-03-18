/*
 * variable_space.h
 * The variable space for the Network Code interpreter
 *  Created on: 2013-03-05
 *      Author: Francis Williams
 */

#include <linux/types.h>
#include <linux/atomic.h>
#include <linux/spinlock.h>
#include <linux/string.h>

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

#define VARSPACE_OK	0

/*
 * Struct representing a single variable
 *
 * NOTE:
 * The data is double buffered. This is done to prevent a possible priority inversion:
 * if a user space thread locks the mutex for writing and gets interrupted, this could block a
 * thread that needs access to the variable space (the NCM interpreter)
 */
struct variable {
	u8			data[2][MAX_VAR_SIZE_BYTES];	/* The (double buffered) data stored in this variable */
	size_t		length[2];						/* The number of bytes of data stored */
	atomic_t	current_buffer;					/* The current buffer being written to */
	rwlock_t	buffer_locks[2];				/* Reader/Writer locks for each buffer */
};

/* The variable space */
typedef struct variable_space {
	struct variable	at[MAX_VARIABLES];
} varspace_t;


/* Initializes a new variable_space */
int init_variable_space(varspace_t* varspace);

/* Gets the data stored for the variable id argument */
int get_variable_data(varspace_t* varspace, u32 var_id, void* out_data, size_t* out_length);

/* Sets the data for the variable id argument to the data argument */
int set_variable_data(varspace_t* varspace, u32 var_id, void* data, size_t length);

#endif /* VARIABLE_SPACE_H_ */
