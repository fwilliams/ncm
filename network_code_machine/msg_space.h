/*
 * msg_space.h
 *
 *  Created on: 2013-03-23
 *      Author: francis
 */

#include <linux/types.h>
#include <linux/spinlock.h>

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
	u8			data[MAX_MESSAGE_SIZE_BYTES];
	u32 		length;
} message_t;

typedef struct message_space {
	message_t	at[MAX_MESSAGES];
} message_space_t;

int init_message_space(message_space_t* msgspace);

int copy_message_from_var(message_space_t* msgspace, varspace_t* varspace, u32 msg_id, u32 var_id);

int copy_message_to_var(message_space_t* msgspace, varspace_t* varspace, u32 msg_id, u32 var_id);

int get_message(message_space_t* msgspace, u32 msg_id, u8* out_data, u32* out_len);

int set_message(message_space_t* msgspace, u32 msg_id, u8* data, u32 len);

#endif /* MSG_SPACE_H_ */
