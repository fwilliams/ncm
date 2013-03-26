/*
 *  hello-1.c - The simplest kernel module.
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <net/net_namespace.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/socket.h>
#include <net/sock.h>
#include <asm-generic/errno-base.h>
#include <net/arp.h>
#include <linux/kthread.h>
#include <linux/completion.h>

#include "nc_net.h"
#include "netcode_helper.h"
#include "msg_space.h"

// the message must have enough (ETH_HLEN) space before it for the header!
static int nc_sendmsg(u8* src_mac, u8 *dest_mac, char* devname, u8 *data, int length) {
	struct net_device* dev;
	int ifindex, ret, len;
	struct socket *sk = NULL;
	struct msghdr msg;
	struct kvec vec;
	/*userdata in ethernet frame*/
	unsigned char* packet;
	/*another pointer to ethernet header*/
	struct ethhdr *eh;
	/* destination*/
	struct sockaddr_ll socket_address;

	if (length > ETH_DATA_LEN) {
		return -1;
	}

	dev = dev_get_by_name(&init_net, devname);
	if (dev == 0) {
		debug_print(KERN_WARNING "failed to find network device\n");
		return -1;
	}
	ifindex = dev->ifindex;
	dev_put(dev);
	debug_print(KERN_INFO "ifindex = %d\n", ifindex);

	packet = data - ETH_HLEN;
	eh = (struct ethhdr *) packet;

	ret = sock_create(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL), &sk);
	if (ret < 0) {
		debug_print(KERN_INFO "sock_create failed");
		return -1;
	}

	/*RAW communication*/
	socket_address.sll_family = PF_PACKET;
	/*we don't use a protocoll above ethernet layer
	 ->just use anything here*/
	socket_address.sll_protocol = htons(ETH_P_IP);
	/*index of the network device*/
	socket_address.sll_ifindex = ifindex;
	/*ARP hardware identifier is ethernet*/
	socket_address.sll_hatype = ARPHRD_ETHER;
	/*target is another host*/
	socket_address.sll_pkttype = PACKET_OTHERHOST;
	/*address length*/
	socket_address.sll_halen = ETH_ALEN;
	/*MAC - begin*/
	socket_address.sll_addr[0] = dest_mac[0];
	socket_address.sll_addr[1] = dest_mac[1];
	socket_address.sll_addr[2] = dest_mac[2];
	socket_address.sll_addr[3] = dest_mac[3];
	socket_address.sll_addr[4] = dest_mac[4];
	socket_address.sll_addr[5] = dest_mac[5];
	/*MAC - end*/
	socket_address.sll_addr[6] = 0x00;/*not used*/
	socket_address.sll_addr[7] = 0x00;/*not used*/

	/*set the frame header*/
	memcpy((void*) eh->h_dest, (void*) dest_mac, ETH_ALEN);
	memcpy((void*) eh->h_source, (void*) src_mac, ETH_ALEN);
	eh->h_proto = ETH_P_NC;

	vec.iov_base = (void *) packet;

	vec.iov_len = length + ETH_HLEN; //ETH_FRAME_LEN;

	msg.msg_name = &socket_address;
	msg.msg_namelen = sizeof(socket_address);
	msg.msg_iov = (struct iovec*) &vec;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
//  MSG_CTRUNC
//	Control data truncated.
//	MSG_DONTROUTE
//	Send without using routing tables.
//	MSG_EOR
//	Terminates a record (if supported by the protocol).
//	MSG_OOB
//	Out-of-band data.
//	MSG_PEEK
//	Leave received data in queue.
//	MSG_TRUNC
//	Normal data truncated.
//	MSG_WAITALL
//	Attempt to fill the read buffer.
	msg.msg_flags = 0;
	len = kernel_sendmsg(sk, &msg, &vec, 1, vec.iov_len);
	debug_print(KERN_INFO "sendmsg ret = %d\n", len);

	// close the socket
	sock_release(sk);
	return 0;
}

static int nc_rcvmsg(struct nc_message *nc_msg, struct socket *sk) {
	struct msghdr msg;
	struct kvec vec;
	int length;

	//necessary preparation
	vec.iov_base = &nc_msg->value;
	vec.iov_len = sizeof(nc_msg->value);
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = (struct iovec*) &vec;
	msg.msg_iovlen = 1;
	// after the message is received, the iov_base pointer moves forward to the end of the message
//	debug_print(KERN_INFO "msg0: %i", vec.iov_base);

	length = kernel_recvmsg(sk, &msg, &vec, 1, vec.iov_len, 0/*MSG_DONTWAIT*/);
	debug_print(KERN_INFO "received length: %i", length);
	debug_print(KERN_INFO "test length: %i", vec.iov_len);

	if (length < 0) {
		debug_print(KERN_INFO "Failed to receive message.");
	} else {
//		debug_print(KERN_INFO "msg1: %i", vec.iov_base);
//		debug_print(KERN_INFO "msg2: %i", nc_msg->value);
//		debug_print(KERN_INFO "msg3: %s", (char*)vec.iov_base);
		debug_print(KERN_INFO "msg4: %s", (char*)nc_msg->value+ETH_HLEN);

		// only consider the receipt a success if it matches our protocol
		if(((struct ethhdr*) nc_msg->value)->h_proto != ETH_P_NC){
			length = -1;
		} else {
			nc_msg->length = length;
		}
	}

	return length;
}

static int receiving_threadfn(void* data) {
	int length, chan, found_channel, ret;
	struct nc_channel *channel;
	struct nc_message *nc_msg;
	struct socket *sk;
	ncm_network_t *ncm_net = (ncm_network_t*) data;

	ret = sock_create(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL), &sk);
	if (ret < 0) {
		debug_print(KERN_INFO "sock_create failed");
		return -1;
	}
	// decreasing this value decreases long it takes in the worst case to unload the module
	// however, it also causes extra overhead - unblocking to check if we should exist early
	sk->sk->sk_rcvtimeo = 1000; // this seems to be about 2-3 seconds

	// get a message buffer ready for the first arrival
	nc_msg = (struct nc_message *) kmalloc(sizeof(struct nc_message),
			GFP_KERNEL);

	while (!kthread_should_stop()) {
		length = nc_rcvmsg(nc_msg, sk);
		debug_print(KERN_INFO "received... (status: %i)", length);
		if (length < 0) {
			debug_print(KERN_INFO "Failed to receive packet (status: %i)", length);
			continue;
		}
		if (kthread_should_stop())
			return -1;

		// check if it's from the right target and has the right type
		found_channel = 0;
		for(chan = 0; chan < MAX_CHANNELS; chan++){
			channel = &ncm_net->at[chan];

			if (memcmp(channel->mac, ((struct ethhdr*) nc_msg->value)->h_source, ETH_ALEN)
					== 0) {
				found_channel = 1;
				debug_print(KERN_INFO "received message: %s", nc_msg->value + ETH_HLEN);

				spin_lock(&channel->lock);
				list_add(&(nc_msg->list), &(channel->message_queue.list));
				spin_unlock(&channel->lock);

				//get a new message buffer ready for the next arrival
				nc_msg = (struct nc_message *) kmalloc(sizeof(struct nc_message),
						GFP_KERNEL);
			}
		}
		if(found_channel == 0){
			debug_print(KERN_INFO "rejected message: %s", nc_msg->value);
		}
	}
	// release the message buffer
	kfree(nc_msg);
	// close the socket
	sock_release(sk);
	return 0;
}



int ncm_receive_message_to_var(ncm_network_t* ncm_net, varspace_t* varspace, u32 chan, u32 var_id){
	struct nc_channel *channel = &(ncm_net->at[chan]);
	int empty;
	struct nc_message *nc_msg;

	spin_lock(&channel->lock);
	empty = list_empty(&channel->message_queue.list);
	spin_unlock(&channel->lock);

	if (empty) {
		debug_print(KERN_INFO "Receive queue empty. Cannot receive.");
		return -1;
	}
	// read the first message off the queue
	spin_lock(&channel->lock);
	nc_msg =
			list_first_entry(&channel->message_queue.list, struct nc_message, list);
	spin_unlock(&channel->lock);

	if (!nc_msg) {
		return -1;
	}

	set_variable_data(varspace, var_id, nc_msg->value+ETH_HLEN, nc_msg->length-ETH_HLEN);

	debug_print(KERN_INFO "RECEIVED: %s", nc_msg->value+ETH_HLEN);

	// delete the message from the queue
	spin_lock(&channel->lock);
	list_del(&nc_msg->list);
	spin_unlock(&channel->lock);
	kfree(nc_msg);
	return 0;
}

void init_network(ncm_network_t* ncm_net, ncm_net_params_t* params){
	int i = 0;
	nc_channel_t* chan;

	for (i = 0; i < MAX_CHANNELS; i++) {
		chan = &(ncm_net->at[i]);
		spin_lock_init(&chan->lock);
		INIT_LIST_HEAD(&chan->message_queue.list);
		memcpy(chan->mac, params->channel_mac[i], ETH_ALEN);
		memcpy(chan->devname, params->net_device_name[i], MAX_DEVNAME_LENGTH);
	}
	memcpy(ncm_net->mac, params->mac_address, ETH_ALEN);
	init_message_space(&ncm_net->message_space);
	ncm_net->receiving_thread = kthread_run(receiving_threadfn, (void*) 0, "NCM network thread");
}

void destroy_network(ncm_network_t* ncm_net) {
	kthread_stop(ncm_net->receiving_thread);
}

//TODO: get rid of extra function call
int ncm_create_message_from_var(ncm_network_t* ncm_net, varspace_t* varspace, u32 var_id, u32 msg_id){
	return copy_message_from_var(&ncm_net->message_space, varspace, msg_id, var_id);
}

int ncm_send_message(ncm_network_t* ncm_net, u32 chan, u32 msg_id){
	nc_channel_t* channel = &(ncm_net->at[chan]);
	return nc_sendmsg(ncm_net->mac, channel->mac, channel->devname, ncm_net->message_space.at[msg_id].data, ncm_net->message_space.at[msg_id].length);
}


/*
int init_module(void) {
	int i;
	unsigned char message[] = "hello there";

	debug_print(KERN_INFO "Start init...\n");
	init_nc_channel_array(chan_array);
	start_receiving();
	ncm_send(0, message, sizeof(message) / sizeof(unsigned char));
	debug_print(KERN_INFO "End init...\n");
	return 0;
}

void cleanup_module(void) {
	ncm_receive(0, 0);
	stop_receiving();
	debug_print(KERN_INFO "Goodbye world 1.\n");
}
*/
