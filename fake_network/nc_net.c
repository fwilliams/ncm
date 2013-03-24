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
#include "nc_net.h"

// http://lxr.free-electrons.com/source/include/linux/skbuff.h#L325

//http://lxr.free-electrons.com/source/net/tipc/eth_media.c#L115
//http://lxr.free-electrons.com/source/net/batman-adv/send.c#L74

// #define MY_BUFF_SIZE 10
// #define MY_MAX_DATA_SIZE 20

// typedef struct{
//   int curr;
//   int last;
//   char * data[MY_BUFF_SIZE];
// }rbuff_tt;

// static int rbuff_set(rbuff_tt *buffer, void * data);
// static void * rbuff_get(rbuff_tt *buffer);
// static void rbuff_init(rbuff_tt *buffer);

// static void * tmp;

// static int rbuff_set(rbuff_tt *buffer, void * data) {

//   buffer->data[buffer->last] = data;
//   buffer->last = ((buffer->last +1) % MY_BUFF_SIZE);
//   return 0;
// }

// static void * rbuff_get(rbuff_tt *buffer){
//   void * tmp;

//   tmp = buffer->data[buffer->curr];
//   buffer->curr = ((buffer->curr +1) % MY_BUFF_SIZE);
//   return tmp;
// }

// static void rbuff_init(rbuff_tt *buff) {
//   buff->curr = 0;
//   buff->last = 0;
// }

// http://www.linuxjournal.com/article/1312
/*
 void append_frame(char *buf, int len)
 {
 struct sk_buff *skb=alloc_skb(len, GFP_ATOMIC);
 if(skb==NULL)
 my_dropped++;
 else
 {
 skb_put(skb,len);
 memcpy(skb->data,data,len);
 skb_append(&my_list, skb);
 }
 }
 void process_queue(void)
 {
 struct sk_buff *skb;
 while((skb=skb_dequeue(&my_list))!=NULL)
 {
 process_data(skb);
 kfree_skb(skb, FREE_READ);
 }
 }
 */

// int num_channels;
// struct nc_client {
//   struct nc_message receive_queue;
//   struct list_head local_clients;
//   int id;
// };
// struct list_head *pos, *q;
// int nc_clients_num;
// struct nc_client local_nc_clients;

void init_nc_channel(struct nc_channel *chan) {
	INIT_LIST_HEAD(&chan->message_queue.list);
}

// 1 - success, 0 - fail, always succeeds in fake network
int nc_channel_send(struct nc_channel *chan, u64 *msg) {
	struct nc_message *tmp;
	tmp = (struct nc_message *) kmalloc(sizeof(struct nc_message), GFP_KERNEL);
	tmp->value = *msg;
	list_add(&(tmp->list), &(chan->message_queue.list));
	return 1;
}

// 1 - success, 0 - fail, always succeeds in fake network
int nc_channel_receive(struct nc_channel *chan, u64 *msg) {
	struct nc_message *nc_msg =
			list_first_entry(&chan->message_queue.list, struct nc_message, list);
	*msg = nc_msg->value;
	list_del(&nc_msg->list);
	kfree(nc_msg);
	return 1;
}


void run_tests(void) {
	struct nc_channel chan;
	u64 msg = 123;
	u64 msg_rx = 123;
	u64 msg2 = 321;
	u64 msg3 = 333;

	init_nc_channel(&chan);

	printk(KERN_INFO "sending: %llu", msg);
	nc_channel_send(&chan, &msg);
	printk(KERN_INFO "sending: %llu", msg2);
	nc_channel_send(&chan, &msg2);
	printk(KERN_INFO "sending: %llu", msg3);
	nc_channel_send(&chan, &msg3);

	nc_channel_receive(&chan, &msg_rx);
	printk(KERN_INFO "receiving: %llu", msg_rx);
	nc_channel_receive(&chan, &msg_rx);
	printk(KERN_INFO "receiving: %llu", msg_rx);
	nc_channel_receive(&chan, &msg_rx);
	printk(KERN_INFO "receiving: %llu", msg_rx);
}

#define NET_DEVICE_NAME "eth0" // "wlan0" // "eth0"  //sometimes enp0s3

int network_rcv_test(void) {
	struct socket *sk = NULL;
	char buffer[ETH_FRAME_LEN]; /*Buffer for ethernet frame*/
	int length = 0; /*length of the received frame*/
	int i, ret, j;
	struct msghdr msg;
	struct kvec vec;

	ret = sock_create(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL), &sk);
	if (ret < 0) {
		printk(KERN_INFO "sock_create failed");
		return -1;
	}

	length = kernel_recvmsg(sk, &msg, &vec, 1, ETH_FRAME_LEN, MSG_DONTWAIT);
	printk(KERN_INFO "received length: %i", length);
	for (i = 0; i < length; i++) {
		printk(KERN_INFO "received: %c", buffer[i]);
	}
	if (length == -1) {
		printk(KERN_INFO "Failed to receive message.");
	}
	// close the socket
	ret = sk->ops->release(sk);
	printk(KERN_INFO "release ret = %d\n", ret);

	if (sk)
		sock_release(sk);
	return 0;
}

int network_send_test(unsigned char* src_mac, unsigned char *dest_mac, unsigned char *message, int length) {

	struct net_device* dev;
	int i,  ifindex, ret, send_result, len;
	struct socket *sk = NULL;
	struct msghdr msg;
	struct kvec vec;
	char buffer[ETH_FRAME_LEN];
	/*pointer to ethenet header*/
	unsigned char* etherhead;
	/*userdata in ethernet frame*/
	unsigned char* data;
	/*another pointer to ethernet header*/
	struct ethhdr *eh;
	/* destination*/
	struct sockaddr_ll socket_address;

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
	for (i = 0; i < sizeof(message); i++) {
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
	len = kernel_sendmsg(sk, &msg, &vec, 1, ETH_FRAME_LEN);
	printk(KERN_INFO "sendmsg ret = %d\n", len);

	// close the socket
	ret = sk->ops->release(sk);
	printk(KERN_INFO "release ret = %d\n", ret);

	if (sk)
		sock_release(sk);
	return 0;
}

int init_module(void) {
	printk(KERN_INFO "Start init...\n");
	unsigned char src_mac[ETH_ALEN] = { 0x08, 0x00, 0x27, 0xC0, 0x56, 0x5C };
	unsigned char dest_mac[ETH_ALEN] = { 0x30, 0x85, 0xA9, 0x8E, 0x87, 0x95 };
	unsigned char message[] = "hello there";
	network_send_test(src_mac, dest_mac, message, sizeof(message));
	network_rcv_test();
	printk(KERN_INFO "End init...\n");
	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "Goodbye world 1.\n");
}
