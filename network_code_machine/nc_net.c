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

// the interface to send on
#define NET_DEVICE_NAME "enp0s3" // "wlan0" // "eth0"  //sometimes enp0s3

unsigned char our_mac[ETH_ALEN] = { 0x08, 0x00, 0x27, 0xC0, 0x56, 0x5B };// original vm
//unsigned char our_mac[ETH_ALEN] = { 0x08, 0x00, 0x27, 0xCf, 0x5c, 0xD7 };// clone vm


/*
struct nc_channel channels[] = { {
//		.mac = { 0x30, 0x85, 0xA9, 0x8E, 0x87, 0x95 } // mac address
//		.mac = { 0x08, 0x00, 0x27, 0xC0, 0x56, 0x5B } // original vm
		.mac = { 0x08, 0x00, 0x27, 0xCf, 0x5c, 0xD7 } // clone vm
//		.mac = { 0x0a, 0x00, 0x27, 0x00, 0x00, 0x00 } // virtual interaface laptop address
} };
*/



static int nc_sendmsg(unsigned char* src_mac, unsigned char *dest_mac,
		unsigned char *message, int length) {

	struct net_device* dev;
	int i, ifindex, ret, len;
	struct socket *sk = NULL;
	struct msghdr msg;
	char buffer[length + ETH_HLEN];
	struct kvec vec;
	/*pointer to ethenet header*/
	unsigned char* etherhead;
	/*userdata in ethernet frame*/
	unsigned char* data;
	/*another pointer to ethernet header*/
	struct ethhdr *eh;
	/* destination*/
	struct sockaddr_ll socket_address;

	if (length > ETH_DATA_LEN) {
		return -1;
	}

	dev = dev_get_by_name(&init_net, NET_DEVICE_NAME);
	if (dev == 0) {
		debug_print(KERN_WARNING "failed to find network device\n");
		return -1;
	}
	ifindex = dev->ifindex;
	dev_put(dev);
	debug_print(KERN_INFO "ifindex = %d\n", ifindex);

	etherhead = buffer;
	data = buffer + ETH_HLEN;
	eh = (struct ethhdr *) etherhead;

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
	memcpy((void*) buffer, (void*) dest_mac, ETH_ALEN);
	memcpy((void*) (buffer + ETH_ALEN), (void*) src_mac, ETH_ALEN);
	eh->h_proto = ETH_P_NC;

	/*fill the frame with some data*/
	for (i = 0; i < length; i++) {
		data[i] = message[i];
	}

	vec.iov_base = (void *) buffer;

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


int ncm_receive(ncm_network_t* ncm_net, u32 chan, u32 var_id) {
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

	// TODO: read value from nc_msg->value of length nc_msg->length into variable var_id
	debug_print(KERN_INFO "RECEIVED: %s", nc_msg->value+ETH_HLEN);

	// delete the message from the queue
	spin_lock(&channel->lock);
	list_del(&nc_msg->list);
	spin_unlock(&channel->lock);
	kfree(nc_msg);
	return 0;
}

void init_network(ncm_network_t* ncm_net){
	int i = 0;
	nc_channel_t* chan = &(ncm_net->at[i]);
	for (i = 0; i < MAX_CHANNELS; i++) {
		spin_lock_init(&chan->lock);
		INIT_LIST_HEAD(&chan->message_queue.list);
	}
	ncm_net->receiving_thread = kthread_run(receiving_threadfn, (void*) 0, "NCM network thread");
}

void destroy_network(ncm_network_t* ncm_net) {
	kthread_stop(ncm_net->receiving_thread);
}

//TODO: remove extra copy by packing the headers when the message is prepared
int ncm_send(ncm_network_t* chan_array, u32 chan, u8 *msg, u32 length) {
	nc_channel_t* channel = &(chan_array->at[chan]);
	if (length > ETH_DATA_LEN) {
		return -1;
	}
	return nc_sendmsg(our_mac, channel->mac, msg, length);
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
