/*
 *  hello-1.c - The simplest kernel module.
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <net/net_namespace.h>
#include <linux/netdevice.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/net.h>
#include "nc_net.h"

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

void network_test(void) {

	struct net_device* dev;
	unsigned char* data;
	unsigned char src[ETH_ALEN] = { 0x08, 0x00, 0x27, 0xC0, 0x56, 0x5B }; // MAC Address of vm
	unsigned char dest[ETH_ALEN] = { 0x52, 0x54, 0x00, 0x12, 0x35, 0x02 }; // Destination MAC
	unsigned char message[] = "hello there";
	struct sk_buff * skb;

	printk(KERN_INFO "hi guise. I aM FAAAKE network driver of frEEEE candy van.");

	skb = alloc_skb(ETH_FRAME_LEN, GFP_KERNEL);
	printk(KERN_INFO "data length: %i, skb->len: %i", skb->data - skb->tail,
			skb->len);

	printk(KERN_INFO "1) skb->truesize %d", skb->truesize);

	skb_reserve(skb, 14);
	printk(KERN_INFO "data length: %i, skb->len: %i", skb->data - skb->tail,
			skb->len);

	printk(KERN_INFO "2) skb->truesize %d", skb->truesize);

	data = skb_put(skb, sizeof(message) / sizeof(char));
	printk(KERN_INFO "data length: %i, skb->len: %i", skb->data - skb->tail,
			skb->len);

	printk(KERN_INFO "3) skb->truesize %d", skb->truesize);

	memcpy(skb->data, message, sizeof(message) / sizeof(char));
	printk(KERN_INFO "skb->data: %s", skb->data);

	// we don't care about network namespaces, so we use init_net
	dev = dev_get_by_index(&init_net, 2);

	if (dev == 0) {
		printk(KERN_INFO "Failed to find network device");
		return;
	}

	printk(KERN_INFO "device %i:", dev);

	printk(
			KERN_INFO "using device: %s: addr 0x%04lx irq %d, MAC addr %pM, MTU:%u\n",
			dev->name, dev->base_addr, dev->irq, dev->dev_addr, dev->mtu);

	skb->dev = dev;
	skb->priority = 0;

	printk(KERN_INFO "hard header return: %i",
			dev_hard_header(skb, dev, ETH_P_802_3, dest, src, skb->len));
	printk(KERN_INFO "data length: %i, skb->len: %i", skb->data - skb->tail,
			skb->len);

	printk(KERN_INFO "4) skb->truesize %d", skb->truesize);


	printk(KERN_INFO "sizeof(struct sk_buff) %d", sizeof(struct sk_buff));

	printk(KERN_INFO "skb->sk %d", skb->sk);

	kfree_skb(skb);
	dev_put(dev);

}

int init_module(void) {
	network_test();

	printk(KERN_INFO "Hello world.\n");

	return 0;
}

void cleanup_module(void) {
	printk(KERN_INFO "Goodbye world 1.\n");
}
