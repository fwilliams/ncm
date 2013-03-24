/*
 *  hello-1.c - The simplest kernel module.
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <net/net_namespace.h>
// #include <linux/netdevice.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/socket.h>
#include <net/arp.h>
#include <linux/syscalls.h>
#include <linux/kthread.h>
#include "nc_net.h"


#define NET_DEVICE_NAME "enp0s3" // "wlan0" // "eth0"  //sometimes enp0s3

unsigned char our_mac[ETH_ALEN] = { 0x08, 0x00, 0x27, 0xC0, 0x56, 0x5B };

#define NUM_CHANNELS sizeof(channels) / sizeof(struct nc_channel)

struct nc_channel channels[] = {
	{
		{ {0,0}, /* kernel's list structure */ 0,/*message*/ 0/*length*/ },
//		{ 0x30, 0x85, 0xA9, 0x8E, 0x87, 0x95 } /*mac address*/
//		{ 0x08, 0x00, 0x27, 0xC0, 0x56, 0x5B } // send to yourself, vm.
		{ 0x0a, 0x00, 0x27, 0x00, 0x00, 0x00 } // virtual interaface laptop address
	}
};

void receiving_threadfn(void* data){
	unsigned char buff[ETH_DATA_LEN];
	int length;
	int channel = (int) data;

	while(!kthread_should_stop()){
		length = nc_rcvmsg(buff, ETH_DATA_LEN);
		printk(KERN_EMERG "received... (status: %i)", length);
		if(length < 0){
			printk(KERN_INFO "Failed to receive packet (status: %i)", length);
		}

		struct nc_message *tmp;
		tmp = (struct nc_message *) kmalloc(sizeof(struct nc_message), GFP_KERNEL);
		memcpy(tmp->value, buff, length);
		tmp->length = length;
		list_add(&(tmp->list), &(channels[channel].message_queue.list));
	}
}

void start_receiving(int channel){
	channels[channel].receiving_thread = kthread_run(receiving_threadfn, (void*) channel, "NCM interpreter");
}

void stop_receiving(int channel) {
	kthread_stop(channels[channel].receiving_thread);
}

void init_nc_channel(struct nc_channel *chan) {
	INIT_LIST_HEAD(&chan->message_queue.list);
}

int nc_channel_send(int chan, unsigned char *msg, int length) {
	struct nc_channel *channel = &channels[chan];
	if(length > ETH_DATA_LEN){
		return -1;
	}
	return nc_sendmsg(&our_mac, &channel->mac, msg, length);
}

int nc_channel_receive(int chan, int var_id) {
	struct nc_channel *channel = &channels[chan];

	if(list_empty(&channel->message_queue.list)){
		return -1;
	}
	// pop the first message off the queue
	struct nc_message *nc_msg = list_first_entry(&channel->message_queue.list, struct nc_message, list);

	if(!nc_msg){
		return -1;
	}
	// TODO: read value from nc_msg->value of length nc_msg->length into variable var_id

	// delete the message from the queue
	list_del(&nc_msg->list);
	kfree(nc_msg);
	return 0;
}


void run_tests(void) {
	struct nc_channel chan;
	u64 msg = 123;
	u64 msg_rx = 123;
	u64 msg2 = 321;
	u64 msg3 = 333;

	init_nc_channel(&chan);

	printk(KERN_INFO "sending: %llu", msg);
	nc_channel_send(0, &msg, 8);
	printk(KERN_INFO "sending: %llu", msg2);
	nc_channel_send(0, &msg2, 8);
	printk(KERN_INFO "sending: %llu", msg3);
	nc_channel_send(0, &msg3, 8);

//	nc_channel_receive(0, &msg_rx);
//	printk(KERN_INFO "receiving: %llu", msg_rx);
//	nc_channel_receive(0, &msg_rx);
//	printk(KERN_INFO "receiving: %llu", msg_rx);
//	nc_channel_receive(0, &msg_rx);
//	printk(KERN_INFO "receiving: %llu", msg_rx);
}


// channels

// queueing




int nc_rcvmsg(unsigned char *buffer, int length) {
	struct socket *sk = NULL;
	int i, ret, j;
	struct msghdr msg;
	char addrbuf[6];
	char controlbuf[20];
	struct kvec vec = {buffer, length};

	/*test*/
	memset(&msg, 0, sizeof(msg));
	msg.msg_name = addrbuf;
	msg.msg_namelen = sizeof(addrbuf);
	msg.msg_iov = &vec;
	msg.msg_iovlen = 1;
	msg.msg_control = controlbuf;
	msg.msg_controllen = sizeof(controlbuf);
	/*test*/

	ret = sock_create(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL), &sk);
	if (ret < 0) {
		printk(KERN_INFO "sock_create failed");
		return -1;
	}

	printk(KERN_EMERG "before...");
	length = kernel_recvmsg(sk, &msg, &vec, 1, length, 0/*MSG_DONTWAIT*/);
	printk(KERN_EMERG "after...");
	printk(KERN_EMERG "received length: %i", length);
//	for (i = 0; i < msg.msg_namelen; i++) {
//		printk(KERN_INFO "%c", addrbuf[i]);
//	}
//	for (i = 0; i < msg.msg_controllen; i++) {
//		printk(KERN_INFO "%c", controlbuf[i]);
//	}
//	for (i = 0; i < length; i++) {
//		printk(KERN_INFO "%c", buffer[i]);
//	}

	if (length == -1) {
		printk(KERN_INFO "Failed to receive message.");
		return -1;
	}
	// close the socket
	ret = sk->ops->release(sk);
	printk(KERN_INFO "release ret = %d\n", ret);

	if (sk)
		sock_release(sk);
	return length;
}

int nc_sendmsg(unsigned char* src_mac, unsigned char *dest_mac, unsigned char *message, int length) {

	struct net_device* dev;
	int i,  ifindex, ret, send_result, len;
	struct socket *sk = NULL;
	struct msghdr msg;
	struct kvec vec;
	/*pointer to ethenet header*/
	unsigned char* etherhead;
	/*userdata in ethernet frame*/
	unsigned char* data;
	/*another pointer to ethernet header*/
	struct ethhdr *eh;
	/* destination*/
	struct sockaddr_ll socket_address;

	if(length > ETH_DATA_LEN){
		return -1;
	}
	char buffer[length + ETH_HLEN];

	dev = dev_get_by_name(&init_net, NET_DEVICE_NAME);
	ifindex = dev->ifindex;
	dev_put(dev);
	printk(KERN_INFO "ifindex = %d\n", ifindex);

	etherhead = buffer;
	data = buffer + ETH_HLEN;
	eh = (struct ethhdr *) etherhead;

	ret = sock_create(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL), &sk);
	if (ret < 0) {
		printk(KERN_INFO "sock_create failed");
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
	eh->h_proto = 0x00;

	/*fill the frame with some data*/
	for (i = 0; i < length; i++) {
		data[i] = message[i];
	}

	vec.iov_base = (void *) buffer;

	vec.iov_len = length + ETH_HLEN;//ETH_FRAME_LEN;

	msg.msg_name = &socket_address;
	msg.msg_namelen = sizeof(socket_address);
	msg.msg_iov = &vec;
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
	printk(KERN_INFO "sendmsg ret = %d\n", len);

	// close the socket
	ret = sk->ops->release(sk);
	printk(KERN_INFO "release ret = %d\n", ret);

	if (sk)
		sock_release(sk);
	return 0;
}

int init_module(void) {
	int i;
	printk(KERN_INFO "Start init...\n");
	for(i = 0; i < NUM_CHANNELS; i++){
		init_nc_channel(&channels[i]);
		start_receiving(i);
	}
	unsigned char message[] = "hello there";
	unsigned char buff[ETH_DATA_LEN];
	nc_channel_send(0, message, sizeof(message)/sizeof(unsigned char));
	printk(KERN_INFO "End init...\n");
	return 0;
}

void cleanup_module(void) {
	int i;
	nc_channel_receive(0, 0);
	for(i = 0; i < NUM_CHANNELS; i++){
		stop_receiving(i);
	}
	printk(KERN_INFO "Goodbye world 1.\n");
}
