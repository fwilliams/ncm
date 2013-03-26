#include <linux/types.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include "msg_space.h"

#ifndef NC_NET_H_
#define NC_NET_H_

#ifndef MAX_CHANNELS
#define MAX_CHANNELS 3
#endif

#ifndef MAX_DEVNAME_LENGTH
#define MAX_DEVNAME_LENGTH 10
#endif

// network code ethernet protocol type
#define ETH_P_NC	0x9009

typedef struct nc_message {
	struct list_head 	list; 					/* kernel's list structure */
	u8 					value[ETH_DATA_LEN];
	u32 				length;
} nc_message_t;

typedef struct nc_channel {
	nc_message_t 	message_queue;
	u8			 	mac[ETH_ALEN];
	spinlock_t 		lock;
	char 			devname[MAX_DEVNAME_LENGTH];
} nc_channel_t;

typedef struct ncm_network {
	nc_channel_t 		at[MAX_CHANNELS];
	struct task_struct* receiving_thread;
	u8					mac[ETH_ALEN];
	message_space_t		message_space;
} ncm_network_t;

typedef struct ncm_net_params {
	u8	net_device_name[MAX_CHANNELS][MAX_DEVNAME_LENGTH];
	u8	mac_address[ETH_ALEN];
	u8	channel_mac[MAX_CHANNELS][ETH_ALEN];
} ncm_net_params_t;

void init_network(ncm_network_t* ncm_net, ncm_net_params_t* params);

void destroy_network(ncm_network_t* ncm_net);

int ncm_send_message(ncm_network_t* ncm_net, u32 chan, u32 msg_id);

int ncm_create_message_from_var(ncm_network_t* ncm_net, varspace_t* varspace, u32 var_id, u32 msg_id);

int ncm_receive_message_to_var(ncm_network_t* ncm_net, varspace_t* varspace, u32 chan, u32 var_id);

#endif /* NC_NET_H_ */
