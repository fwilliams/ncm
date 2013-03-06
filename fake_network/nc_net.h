#include <linux/types.h>
#include <linux/list.h>

struct nc_message {
  struct list_head list; /* kernel's list structure */
  u64 value;
};

struct nc_channel {
  struct nc_message message_queue;
};

void init_nc_channel(struct nc_channel *chan);

// 1 - success, 0 - fail, always succeeds in fake network
int nc_channel_send(struct nc_channel *chan, u64 *msg);

// 1 - success, 0 - fail, always succeeds in fake network
int nc_channel_receive(struct nc_channel *chan, u64 *msg);