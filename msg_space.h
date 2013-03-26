/*
 * msg_space.h
 *
 *  Created on: 2013-03-23
 *      Author: francis
 */

#include <linux/types.h>
#include <linux/spinlock.h>
#include <linux/if_ether.h>

#include "variable_space.h"

#ifndef MSG_SPACE_H_
#define MSG_SPACE_H_

/*
 * Maximum number of bytes per message
 */
#ifndef MAX_MESSAGE_SIZE_BYTES
#define MAX_MESSAGE_SIZE_BYTES 1500
#endif

/*
 * Maximum number messages
 */
#ifndef MAX_MESSAGES
#define MAX_MESSAGES 10
#endif

#define MSG_SPACE_OK 0

typedef struct message {
	rwlock_t	lock;
	u8			buff[ETH_FRAME_LEN];
	u8			*data;					// points to the beginning of the actual data inside buff (past the header)
	u32 		length;					// the length of the data, not the whole buffer
} message_t;

typedef struct message_space {
	message_t	at[MAX_MESSAGES];
} message_space_t;

int init_message_space(message_space_t* msgspace);

int copy_message_from_var(message_space_t* msgspace, varspace_t* varspace, u32 msg_id, u32 var_id);

#endif /* MSG_SPACE_H_ */
