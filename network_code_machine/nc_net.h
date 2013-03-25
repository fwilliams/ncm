#include <linux/types.h>
#include <linux/list.h>
#include <linux/if_ether.h>

#ifndef MAX_CHANNELS
#define MAX_CHANNELS 3
#endif

typedef struct nc_message {
	struct list_head 	list; 					/* kernel's list structure */
	unsigned char 		value[ETH_DATA_LEN];
	int 				length;
} nc_message_t;

typedef struct nc_channel {
	nc_message_t 	message_queue;
	unsigned char 	mac[ETH_ALEN];
	spinlock_t 		lock;
} nc_channel_t;

typedef struct nc_chan_array {
	nc_channel_t 	at[MAX_CHANNELS];
} nc_channel_array_t;

void init_nc_channel_array(nc_channel_array_t* chan_array);

void destroy_nc_channel_array(nc_channel_array_t* chan_array);

int nc_channel_send(nc_channel_array_t* chan_array, u32 chan, u8 *msg, u32 length);

int nc_channel_receive(nc_channel_array_t* chan_array, u32 chan, u32 var_id);

void start_receiving();

void stop_receiving();
