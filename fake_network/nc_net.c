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
#include <net/sock.h>
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
  tmp = (struct nc_message *)kmalloc(sizeof(struct nc_message), GFP_KERNEL);
  tmp->value = *msg;
  list_add(&(tmp->list), &(chan->message_queue.list));
  return 1;
}

// 1 - success, 0 - fail, always succeeds in fake network
int nc_channel_receive(struct nc_channel *chan, u64 *msg) {
  struct nc_message *nc_msg = list_first_entry(&chan->message_queue.list, struct nc_message, list);
  *msg = nc_msg->value;
  list_del(&nc_msg->list);
  kfree(nc_msg);
  return 1;
}

// // Returns an id that will be used to identify this client when
// // sending or receiving data
// int init_nc_client(struct nc_client *tmp) {
//   // struct nc_client *tmp;
//   /* adding elements to mylist */
//   // tmp= (struct nc_client *)kmalloc(sizeof(struct nc_client), GFP_KERNEL);//TODO: am I allocating memory correctly?
  
//   /* INIT_LIST_HEAD(&tmp->list); 
//    *
//    * this initializes a dynamically allocated list_head. we
//    * you can omit this if subsequent call is add_list() or 
//    * anything along that line because the next, prev
//    * fields get initialized in those functions.
//    */
//    //TODO: increment this number
//   tmp->id = nc_clients_num++;

//   /* add the new item 'tmp' to the list of items in mylist */
//   list_add(&(tmp->local_clients), &(local_nc_clients.local_clients));
//   /* you can also use list_add_tail() which adds new items to
//    * the tail end of the list
//    */
//   return nc_clients_num-1;
// }

// void print_nc_client_list() {
//   struct nc_client *tmp;

//   printk(KERN_INFO "nc client list:");
//   list_for_each(pos, &local_nc_clients.local_clients) {
//     tmp = list_entry(pos, struct nc_client, local_clients);
//     printk(KERN_INFO "client id: %i", tmp->id);
//   }
// }

// void destroy_nc_client(struct nc_client *tmp) {
//   list_del(&tmp->local_clients);
//   nc_clients_num--;
// }

void run_tests(void){
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

/*receive:
void* buffer = (void*)malloc(ETH_FRAME_LEN); /*Buffer for ethernet frame*
int length = 0; /*length of the received frame*
...
length = recvfrom(s, buffer, ETH_FRAME_LEN, 0, NULL, NULL);
if (length == -1) { errorhandling .... }
*/

void network_test(void){

  struct net_device* dev;
//  unsigned char* data;
  unsigned char src_mac[ETH_ALEN] = {0x08,0x00,0x27,0xC0,0x56,0x5C};
  unsigned char dest_mac[ETH_ALEN] = {0x52,0x54,0x00,0x12,0x35,0x02};
//  unsigned char dest[ETH_ALEN] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
  unsigned char message[] = "hello there";
  struct sk_buff * skb;
  int i;
  int ret;
  int send_result;
  struct socket *sk = NULL;

	struct msghdr msg;
	struct kvec vec;
	int len;
  int s;//socket file discriptor
  // struct sock *sk = NULL;
  void* buffer;
/*pointer to ethenet header*/
unsigned char* etherhead;
/*userdata in ethernet frame*/
unsigned char* data;
/*another pointer to ethernet header*/
struct ethhdr *eh;
/* destination*/
  struct sockaddr_ll socket_address;

buffer = (void*)kmalloc(ETH_FRAME_LEN, GFP_KERNEL );
etherhead = buffer;
data = buffer + 14;
eh = (struct ethhdr *)etherhead;

  printk(KERN_INFO "hi guise. I aM FAAAKE network driver of frEEEE candy van.");
  // dev = dev_get_by_index(&init_net, 0);


	ret = sock_create(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL), &sk);
	if (ret < 0)
	{
		printk(KERN_INFO "sock_create failed");
		return;
	}

/*RAW communication*/
  socket_address.sll_family   = PF_PACKET;
/*we don't use a protocoll above ethernet layer
  ->just use anything here*/
  socket_address.sll_protocol = htons(ETH_P_IP);
/*index of the network device*/
  socket_address.sll_ifindex  = 2;//TODO: get dynamically?
/*ARP hardware identifier is ethernet*/
  socket_address.sll_hatype   = ARPHRD_ETHER;
/*target is another host*/
socket_address.sll_pkttype  = PACKET_OTHERHOST;
/*address length*/
socket_address.sll_halen    = ETH_ALEN;   
/*MAC - begin*/
socket_address.sll_addr[0]  = dest_mac[0];   
socket_address.sll_addr[1]  = dest_mac[1];   
socket_address.sll_addr[2]  = dest_mac[2];
socket_address.sll_addr[3]  = dest_mac[3];
socket_address.sll_addr[4]  = dest_mac[4];
socket_address.sll_addr[5]  = dest_mac[5];
/*MAC - end*/
socket_address.sll_addr[6]  = 0x00;/*not used*/
socket_address.sll_addr[7]  = 0x00;/*not used*/


/*set the frame header*/
memcpy((void*)buffer, (void*)dest_mac, ETH_ALEN);
memcpy((void*)(buffer+ETH_ALEN), (void*)src_mac, ETH_ALEN);
eh->h_proto = 0x00;


/*fill the frame with some data*/
for (i = 0; i < 1500; i++) {
  data[i] = 8;
}

/*send the packet*/
// send_result = sys_sendto(s, buffer, ETH_FRAME_LEN, 0,
//         (struct sockaddr*)&socket_address, sizeof(socket_address));
// if (send_result == -1) {
// 	printk(KERN_INFO "error!");
/*errorhandling...* }

/*

  // set up src
	struct sockaddr srcaddr;
	srcaddr.sa_family = AF_UNSPEC;
	memcpy(srcaddr.sa_data, src_mac, ETH_ALEN);
	ret = sk->ops->bind(sk, &srcaddr, ETH_ALEN+100000);
	printk(KERN_INFO "bind ret = %d\n", ret);


	// set up dest

	struct sockaddr dstaddr;
	dstaddr.sa_family = AF_UNSPEC;
	memcpy(dstaddr.sa_data, dest_mac, ETH_ALEN);
	ret = sk->ops->connect(sk, &dstaddr, ETH_ALEN, 0);
	printk(KERN_INFO "connect ret = %d\n", ret);
*/
	// send

	vec.iov_base = (void *)buffer;

	vec.iov_len = ETH_FRAME_LEN;
//	vec.iov_len = (__kernel_size_t)(sizeof(buffer)/sizeof(unsigned char));

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
  len = kernel_sendmsg(sk, &msg,
       &vec, 1, ETH_FRAME_LEN);
//	len = sock_sendmsg(sk, &msg, sizeof(message)/sizeof(unsigned char));//?
	printk(KERN_INFO "sendmsg ret = %d\n", len);

	// close the socket
	ret = sk->ops->release(sk);
	printk(KERN_INFO "release ret = %d\n", ret);

	if (sk)
		sock_release(sk);


/*
  skb = alloc_skb(ETH_FRAME_LEN, GFP_KERNEL);
  printk(KERN_INFO "data length: %i, skb->len: %i", skb->data - skb->tail, skb->len); 
  printk(KERN_INFO "What's the diff? %i", skb->head - skb->data);

  skb_reserve(skb,14);
  printk(KERN_INFO "data length: %i, skb->len: %i", skb->data - skb->tail, skb->len);
  printk(KERN_INFO "What's the diff? %i", skb->head - skb->data);
  // skb_put(skb,ETH_FRAME_LEN);

  data = skb_put(skb,sizeof(message)/sizeof(char));
  printk(KERN_INFO "data length: %i, skb->len: %i", skb->data - skb->tail, skb->len);
  printk(KERN_INFO "What's the diff? %i", skb->head - skb->data);
  // int err = 0;
  // skb->csum = csum_and_copy_from_user(&message, data,
  //             sizeof(message)/sizeof(char), 0, &err);
  // if(err){
  //   printk(KERN_INFO "err: %i", err);
  // }
  memcpy(skb->data,message,sizeof(message)/sizeof(char));
  printk(KERN_INFO "skb->data: %s", skb->data);
  // skb->data_len += sizeof(message)/sizeof(char);
  // skb->len += sizeof(message)/sizeof(char);

  // we don't care about network namespaces, so we use init_net
//  dev = dev_get_by_name(&init_net, "eth0"); //sometimes enp0s3
   dev = dev_get_by_index(&init_net, 2);

  if(dev == 0){
    printk(KERN_INFO "Failed to find network device");
    return;
  }
  printk(KERN_INFO "device %i:", dev);

  printk(KERN_INFO "using device: %s: addr 0x%04lx irq %d, MAC addr %pM, MTU:%u\n",
     dev->name, dev->base_addr, dev->irq, dev->dev_addr, dev->mtu);

  skb->dev = dev;
  skb->priority = 1;
  printk(KERN_INFO "What's the diff? %i", skb->head - skb->data);
//  printk(KERN_INFO "wtf-1");
//  i=0;
//  while(i < 26){
//	  printk(KERN_INFO "%i:%c", skb->head + i, skb->head[i]);
//	  i++;
//  }
//  printk(KERN_INFO "wtf0");
//  i=0;
//  while(i < 26){
//	  printk(KERN_INFO "%i:%c", skb->data + i, skb->data[i]);
//	  i++;
//  }
//
  printk(KERN_INFO "hard header return: %i", dev_hard_header(skb, dev, ETH_P_802_2, dest, src, skb->len));
  printk(KERN_INFO "data length: %i, skb->len: %i", skb->data - skb->tail, skb->len);
  printk(KERN_INFO "What's the diff? %i", skb->head - skb->data);
//  printk(KERN_INFO "wtf1");
//  i=0;
//  while(i < 26){
//	  printk(KERN_INFO "%i:%c", skb->head + i, skb->head[i]);
//	  i++;
//  }
//  printk(KERN_INFO "wtf2");
//  i=0;
//  while(i < 26){
//	  printk(KERN_INFO "%i:%c", &skb->data + i, skb->data[i]);
//	  i++;
//  }
//  skb->hdr_len = 14;//not sure what this might achieve
//  skb->pkt_type = ETH_P_802_3;//not sure what this might achieve; not sure if this is the right type of flag for here
  printk(KERN_INFO "hdr_len: %i", skb->hdr_len);
  printk(KERN_INFO "pkt_type: %i", skb->pkt_type);


  sk = sk_alloc(&init_net, PF_PACKET, GFP_KERNEL);
  skb->sk = sk;

  if(dev_queue_xmit(skb)!=NET_XMIT_SUCCESS)
  {
      printk("Not sent!!\n");
  }

  sk_free(sk);
  kfree_skb(skb);
  dev_put(dev);
*/
}

int init_module(void)
{
  network_test();
/*

  printk(KERN_INFO "test: %i", dev->addr_len);
  skb = alloc_skb(ETH_FRAME_LEN+sizeof(message)/sizeof(char),GFP_KERNEL);   
  skb->dev = dev;

  skb_reserve(skb,ETH_FRAME_LEN);
  // skb_put(skb,ETH_FRAME_LEN);


  skbdata = skb_put(skb, sizeof(message)/sizeof(char));
  err = 0;
  skb->csum = csum_and_copy_from_user(&message, skbdata,
              sizeof(message)/sizeof(char), 0, &err);
  if (err)
    printk(KERN_INFO "broke while trying to add data");

*/



 // fill out ethernet header,skb->mac_header,skb->data, skb->dev
  // int dev_queue_xmit (struct sk_buff * skb);
/*
  //dev->hard_start_xmit( //<- this is the real transmit

  skb = alloc_skb(123, GFP_KERNEL);
  skb->data
  skb_mac_header(skb) // to use the union data struct and cast it to mac_header
  //kfree_skb()

  skb=alloc_skb(len+headspace, GFP_KERNEL);
  skb_reserve(skb, headspace);
  skb_put(skb,len);
  memcpy_fromfs(skb->data,data,len);
  pass_to_m_protocol(skb);

  skb=sock_alloc_send_skb(sk,....)
  if(skb==NULL)
      return -err;
  skb->sk=sk;
  skb_reserve(skb, headroom);
  skb_put(skb,len);
  memcpy(skb->data, data, len);
  protocol_do_something(skb);

  */
 // 86         GFP_KERNEL,
 // 87         GFP_ATOMIC,
  // dev->netdev_ops->
  
// dev_loopback_xmit(struct sk_buff *newskb);
 // dev_queue_xmit(struct sk_buff *skb);

// adapter->netdev->netdev_ops->e1000_xmit_frame
//   netdev_tx_t 
//   e1000_xmit_frame(struct sk_buff *skb,
//                               struct net_device *netdev)
  // struct net_device *dev_get_by_index(struct net *net, int ifindex)
  // struct net* nnet;

  printk(KERN_INFO "Hello world.\n");

  // int dev_queue_xmit (struct sk_buff * skb);
  // e1000_xmit_frame((struct sk_buff *)NULL, (struct net_device *)NULL);
	/*
	 * A non 0 return means init_module failed; module can't be loaded.
	 */
	return 0;
}

void cleanup_module(void)
{
	printk(KERN_INFO "Goodbye world 1.\n");
}
