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
#include <linux/rcupdate.h>

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
 * Default name of character device used to write to variable space
 */
#ifndef VARSPACE_CHRDEV_NAME
#define VARSPACE_CHRDEV_NAME "ncm_varspace"
#endif


#define VARSPACE_OK	0

/*
 * Struct representing a single variable
 */
typedef struct var_data {
	u8			data[MAX_VAR_SIZE_BYTES];	/* The data stored in this variable */
	size_t		length;						/* The number of bytes of data stored */
} vardata_t;

typedef struct variable {
	vardata_t	varbuf[2];	/* Double buffer of variables */
	vardata_t*	read_buf;	/* Pointer to the front buffer */
	vardata_t*	write_buf;	/* Pointer to the back buffer */
	spinlock_t	write_lock;	/* Lock on the write buffer pointer */
} variable_t;


/* The variable space */
typedef struct variable_space {
	variable_t	at[MAX_VARIABLES];	/* Array of all variables available */
} varspace_t;


/* Initializes a new variable_space */
int init_variable_space(varspace_t* varspace);

/* Restores all ressources used by the variable space */
int destroy_variable_space(varspace_t* varspace);

/* Gets the data stored for the variable id argument */
int get_variable_data(varspace_t* varspace, u32 var_id, u8* out_data, size_t* out_length);

/* Sets the data for the variable id argument to the data argument */
int set_variable_data(varspace_t* varspace, u32 var_id, u8* data, size_t length);

/*
 * Returns 0 if two variables are equal, returns a positive number if var_1 > var_2, and
 * returns a negative number if var_1 < var_2.
 *
 * NOTE: Variable comparison is done via byte-wise subtraction: var_1[i] - var_2[i]
 * until a non-zero result is found. The sign of the result
 */
int cmp_variables(varspace_t* varspace, u32 var_id_1, u32 var_id_2);

#endif /* VARIABLE_SPACE_H_ */
