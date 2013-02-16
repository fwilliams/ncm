/*
 *  hello-1.c - The simplest kernel module.
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <net/net_namespace.h>
#include <linux/netdevice.h>
// #include <linux/types.h>

struct sk_buff; // http://lxr.free-electrons.com/source/include/linux/skbuff.h#L325

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

int init_module(void)
{
  struct net_device* dev;
  struct sk_buff* data;
  unsigned char src[ETH_ALEN] = {0x08,0x00,0x27,0xC0,0x56,0x5B};
  unsigned char dest[ETH_ALEN] = {0x52,0x54,0x00,0x12,0x35,0x02};
  unsigned char message[] = "hello there";
  struct sk_buff * skb;

  printk(KERN_INFO "hi guise. I aM FAAAKE network driver of frEEEE candy van.");
  // dev = dev_get_by_index(&init_net, 0);
  
  dev = dev_get_by_name(&init_net, "enp0s3");

  printk(KERN_INFO "using device: %s: addr 0x%04lx irq %d, MAC addr %pM, MTU:%u\n",
     dev->name, dev->base_addr, dev->irq, dev->dev_addr, dev->mtu);


  printk(KERN_INFO "test: %i", dev->addr_len);
  skb = alloc_skb(ETH_FRAME_LEN+sizeof(message)/sizeof(char),GFP_KERNEL);   
  skb->dev = dev;

  skb_reserve(skb,ETH_FRAME_LEN);
  // skb_put(skb,ETH_FRAME_LEN);

  skb_put(skb,sizeof(message)/sizeof(char));
  memcpy(skb->data,message,sizeof(message)/sizeof(char));
  // skb->data_len = sizeof(message)/sizeof(char);

  printk(KERN_INFO "hard hader return: %i", dev_hard_header(skb, dev, ETH_P_802_3, dest, src, dev->addr_len));

  if(dev_queue_xmit(skb)!=NET_XMIT_SUCCESS)
  {
      printk("Not send!!\n");
  }

  kfree_skb(skb);

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

  printk(KERN_INFO "Hello world 2.\n");

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
