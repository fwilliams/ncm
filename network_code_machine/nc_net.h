#include <linux/types.h>
#include <linux/list.h>
#include <linux/if_ether.h>

#ifndef NC_NET_H_
#define NC_NET_H_

#ifndef MAX_CHANNELS
#define MAX_CHANNELS 3
#endif

// network code ethernet protocol type
#define ETH_P_NC	0x9009

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

typedef struct ncm_network {
	nc_channel_t 		at[MAX_CHANNELS];
	struct task_struct* receiving_thread;
} ncm_network_t;

void init_network(ncm_network_t* ncm_net);

void destroy_network(ncm_network_t* ncm_net);

int ncm_send(ncm_network_t* ncm_net, u32 chan, u8 *msg, u32 length);

int ncm_receive(ncm_network_t* ncm_net, u32 chan, u32 var_id);

#endif /* NC_NET_H_ */
