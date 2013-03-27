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
#include <linux/jiffies.h>

#include "nc_net.h"
#include "netcode_helper.h"


// the message must have enough (ETH_HLEN) space before it for the header!
static int nc_sendmsg(u8* src_mac, u8 *dest_mac, struct socket *sk, int ifindex, u8 *data, int length, int protocol) {
	int len;
	struct msghdr msg;
	struct kvec vec;
	/*userdata in ethernet frame*/
	unsigned char* packet;
	/*another pointer to ethernet header*/
	struct ethhdr *eh;
	/* destination*/
	struct sockaddr_ll socket_address;

	if (length > ETH_DATA_LEN){
		debug_print(KERN_WARNING "Attempting to send a packet that exceeds the size of an ethernet frame.");
		return -NC_ETOOBIG;
	}
	if(ifindex <= 0){
		debug_print(KERN_WARNING "Attempting to send a packet over a channel with no network interface configured.");
		return -NC_ENOIF;
	}

	packet = data - ETH_HLEN;
	eh = (struct ethhdr *) packet;


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
	eh->h_proto = protocol;

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

	return 0;
}

static int nc_rcvmsg(u8 *buff, int bufflen, struct socket *sk, int protocol) {
	struct msghdr msg;
	struct kvec vec;
	int length;

	//necessary preparation
	vec.iov_base = buff;
	vec.iov_len = bufflen;
	memset(&msg, 0, sizeof(msg));
	msg.msg_iov = (struct iovec*) &vec;
	msg.msg_iovlen = 1;
	// after the message is received, the iov_base pointer moves forward to the end of the message
//	debug_print(KERN_INFO "msg0: %i", vec.iov_base);

	length = kernel_recvmsg(sk, &msg, &vec, 1, vec.iov_len, 0/*MSG_DONTWAIT*/);
//	debug_print(KERN_INFO "received length: %i", length);
//	debug_print(KERN_INFO "test length: %i", vec.iov_len);

	if (length < 0) {
//		debug_print(KERN_INFO "No message.");
	} else {

		// only consider the receipt a success if it matches our protocol
		if(((struct ethhdr*) buff)->h_proto != protocol){
			length = NC_ENOTNC;
		} else {
			//		debug_print(KERN_INFO "msg1: %i", vec.iov_base);
			//		debug_print(KERN_INFO "msg2: %i", nc_msg->value);
			//		debug_print(KERN_INFO "msg3: %s", (char*)vec.iov_base);
					debug_print(KERN_INFO "msg4: %s", (char*)buff + ETH_HLEN);
		}
	}

	return length;
}

static int receiving_threadfn(void* data) {
	int length, chan, found_channel;
	struct nc_channel *channel;
	struct nc_message *nc_msg;
	ncm_network_t *ncm_net = (ncm_network_t*) data;

	// decreasing this value decreases long it takes in the worst case to unload the module
	// however, it also causes extra overhead - unblocking to check if we should exist early
	ncm_net->receive_socket->sk->sk_rcvtimeo = 1000; // this seems to be about 2-3 seconds

	// get a message buffer ready for the first arrival
	nc_msg = (struct nc_message *) kmalloc(sizeof(struct nc_message),
			GFP_KERNEL);

	while (!kthread_should_stop()) {
		length = nc_rcvmsg(nc_msg->value, sizeof(nc_msg->value), ncm_net->receive_socket, ETH_P_NC);
//		debug_print(KERN_INFO "received... (status: %i)", length);
		if (length < 0) {
//			debug_print(KERN_INFO "No packet received (status: %i)", length);
			continue;
		} else {
			nc_msg->length = length;
		}
		if (kthread_should_stop())
			goto out;

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
out:
	// release the message buffer
	kfree(nc_msg);
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
		debug_print(KERN_WARNING "Receive queue empty. Cannot receive.");
		return -NC_ENOMSG;
	}
	// read the first message off the queue
	spin_lock(&channel->lock);
	nc_msg =
			list_first_entry(&channel->message_queue.list, struct nc_message, list);
	spin_unlock(&channel->lock);

	// redundant check
	if (!nc_msg) {
		debug_print(KERN_WARNING "Receive queue empty. Cannot receive.");
		return -NC_ENOMSG;
	}

	// copy the entire contents of the message (excluding ethernet headers)
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
	int i, ret;
	struct net_device *dev;
	nc_channel_t* chan;

	for (i = 0; i < MAX_CHANNELS; i++) {
		chan = &(ncm_net->at[i]);
		spin_lock_init(&chan->lock);
		INIT_LIST_HEAD(&chan->message_queue.list);
		memcpy(chan->mac, params->channel_mac[i], ETH_ALEN);
		ncm_net->sync_packetlen = ETH_ZLEN;
		dev = dev_get_by_name(&init_net, params->net_device_name[i]);
		if (dev == 0) {
			debug_print(KERN_WARNING "failed to find network device!!!!! %s\n", params->net_device_name[i]);
			chan->ifindex = -1;
		} else {
			chan->ifindex = dev->ifindex;
			dev_put(dev);
		}

		ret = sock_create(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL), &chan->send_socket);
		if (ret < 0) {
			debug_print(KERN_WARNING "sock_create failed for channel %i", i);
		}
	}

	ret = sock_create(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL), &ncm_net->receive_socket);
	if (ret < 0) {
		debug_print(KERN_WARNING "sock_create failed on receive socket");
	}
	memcpy(ncm_net->mac, params->mac_address, ETH_ALEN);

	for(i = 0; i < MAX_MESSAGES; i++) {
		rwlock_init(&ncm_net->message_space.at[i].lock);
		ncm_net->message_space.at[i].length = 0;
		ncm_net->message_space.at[i].data = ncm_net->message_space.at[i].buff + ETH_HLEN;
	}

	ncm_net->receiving_thread = kthread_run(receiving_threadfn, (void*) ncm_net, "NCM network thread");
}

void destroy_network(ncm_network_t* ncm_net) {
	int i;
	nc_channel_t* chan;
	kthread_stop(ncm_net->receiving_thread);
	for (i = 0; i < MAX_CHANNELS; i++) {
		chan = &(ncm_net->at[i]);
		sock_release(chan->send_socket);
	}
	sock_release(ncm_net->receive_socket);
}

int ncm_create_message_from_var(ncm_network_t* ncm_net, varspace_t* varspace, u32 var_id, u32 msg_id){
	write_lock(&ncm_net->message_space.at[msg_id].lock);
	get_variable_data(varspace, var_id, ncm_net->message_space.at[msg_id].data, &ncm_net->message_space.at[msg_id].length);
	write_unlock(&ncm_net->message_space.at[msg_id].lock);
	return 0;
}

int ncm_send_message(ncm_network_t* ncm_net, u32 chan, u32 msg_id){
	nc_channel_t* channel = &(ncm_net->at[chan]);
	return nc_sendmsg(ncm_net->mac, channel->mac, channel->send_socket, channel->ifindex, ncm_net->message_space.at[msg_id].data, ncm_net->message_space.at[msg_id].length, ETH_P_NC);
}

int ncm_send_sync(ncm_network_t* ncm_net, u32 chan){
	nc_channel_t* channel = &(ncm_net->at[chan]);
	return nc_sendmsg(ncm_net->mac, channel->mac, channel->send_socket, channel->ifindex, ncm_net->sync_packet + ETH_HLEN, ncm_net->sync_packetlen - ETH_HLEN, ETH_P_NC_SYNC);
}

// timeout is in milliseconds
int ncm_receive_sync(ncm_network_t* ncm_net, int timeout){
	int length;
	u64 start, now;
	u8 buff[ncm_net->sync_packetlen];
	start = now_us();

	// decreasing this value decreases long it takes in the worst case to unload the module
	// however, it also causes extra overhead - unblocking to check if we should exist early
	now = now_us();
	// convert timer from jiffies to to microseconds
	ncm_net->receive_socket->sk->sk_rcvtimeo = usecs_to_jiffies(start - now + timeout);

	while (ncm_net->receive_socket->sk->sk_rcvtimeo > 0) {
		length = nc_rcvmsg(buff, ncm_net->sync_packetlen, ncm_net->receive_socket, ETH_P_NC_SYNC);
		if (length < 0) {
			debug_print(KERN_INFO "Failed to sync (status: %i)", length);
		} else {
			debug_print(KERN_INFO "CLINET SYNCED %i", length);
		}
		now = now_us();
		ncm_net->receive_socket->sk->sk_rcvtimeo = usecs_to_jiffies(start - now + timeout);
	}
	return 0;
}
