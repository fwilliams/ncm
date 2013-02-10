/*
 *  hello-1.c - The simplest kernel module.
 */
#include <linux/module.h>	/* Needed by all modules */
#include <linux/kernel.h>	/* Needed for KERN_INFO */
#include <net/net_namespace.h>
#include <linux/netdevice.h>

struct sk_buff; // http://lxr.free-electrons.com/source/include/linux/skbuff.h#L325

int init_module(void)
{
  struct net_device* device;
  
	printk(KERN_INFO "Hello world 111111.\n");

// adapter->netdev->netdev_ops->e1000_xmit_frame
//   netdev_tx_t 
//   e1000_xmit_frame(struct sk_buff *skb,
//                               struct net_device *netdev)
  // struct net_device *dev_get_by_index(struct net *net, int ifindex)
  // struct net* nnet;

  device = dev_get_by_index(&init_net, 0);
  printk(KERN_INFO "Hello world 2.\n");
  netdev_info(device, "testing!");
  printk(KERN_INFO "Hello world 3.\n");
  // int netdev_info(const struct net_device *dev, const char *format, ...);

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
