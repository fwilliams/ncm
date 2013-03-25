#include <linux/types.h>
#include <linux/list.h>
#include <linux/if_ether.h>

struct nc_message {
  struct list_head list; /* kernel's list structure */
  unsigned char value[ETH_DATA_LEN];
  int length;
};

struct nc_channel {
  struct nc_message message_queue;
  unsigned char mac[ETH_ALEN];
  spinlock_t lock;
};

void init_nc_channel(struct nc_channel *chan);

int nc_channel_send(int chan, unsigned char *msg, int length);

int nc_channel_receive(int chan, int var_id);
