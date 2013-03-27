#include <linux/types.h>
#include <linux/list.h>
#include <linux/if_ether.h>
#include "variable_space.h"

#ifndef NC_NET_H_
#define NC_NET_H_

#ifndef MAX_CHANNELS
#define MAX_CHANNELS 1
#endif

#ifndef MAX_DEVNAME_LENGTH
#define MAX_DEVNAME_LENGTH 10
#endif

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

// network code ethernet protocol type
#define ETH_P_NC		0x9009
#define ETH_P_NC_SYNC 	0x900A

// possible network errors
#define NC_ENOMSG 		35 // no message received
#define NC_ENOIF 		36 // no network interface available
#define NC_ETOOBIG	 	37 // the message to send is bigger than an ethernet frame
#define NC_ENOTNC 		38 // the packet received was not a network code packet
#define NC_ENOTUS 		39 // the packet received was not for us

typedef struct message {
	rwlock_t	lock;
	u8			buff[ETH_FRAME_LEN];
	u8			*data;					// points to the beginning of the actual data inside buff (past the header)
	u32 		length;					// the length of the data, not the whole buffer
} message_t;

typedef struct message_space {
	message_t	at[MAX_MESSAGES];
} message_space_t;

typedef struct nc_message {
	struct list_head 	list; 					/* kernel's list structure */
	u8 					value[ETH_DATA_LEN];
	u32 				length;
} nc_message_t;

typedef struct nc_channel {
	nc_message_t 	message_queue;
	u8			 	mac[ETH_ALEN];
	spinlock_t 		lock;
	u32				ifindex;
	struct socket*	send_socket;
} nc_channel_t;

typedef struct ncm_network {
	nc_channel_t 		at[MAX_CHANNELS];
	struct task_struct* receiving_thread;
	u8					mac[ETH_ALEN];
	message_space_t		message_space;
	u8					sync_packet[ETH_ZLEN];
	u8					sync_packetlen; // = ETH_ZLEN (includes ETH_HLEN)
	struct socket*		receive_socket;
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

int ncm_send_sync(ncm_network_t* ncm_net, u32 chan);

int ncm_receive_sync(ncm_network_t* ncm_net, int timeout);

#endif /* NC_NET_H_ */
