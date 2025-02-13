#ifndef Y11__S_DYNFD_H
#define Y11__S_DYNFD_H

#include <stdint.h>

typedef struct data_queue_entry data_queue_entry_t;
typedef struct data_queue data_queue_t;

struct dynfd;

typedef void eventfd_handler_f(uint32_t events, struct dynfd* dfd);
typedef void eventfd_destroy_f(struct dynfd* dfd);

enum {
  DQ_INITIAL_CAPACITY     =        64,
  DQ_PAGE_LIMIT           =      4096,
  DQ_MESSAGE_MEMORY_LIMIT = 1024*1024,
};

struct data_queue_entry {
  struct data_queue_entry* next;
  short size, used;
  char data[];
};

struct data_queue {
  struct data_queue_entry *first, **last; // Last should be initially set to &first. For simplicity, this is done in add_fd for now.
  unsigned size;
  unsigned short offset, count;
};

extern struct dynfd* post_process_list;

struct dynfd_type {
  eventfd_handler_f* ondata;
  eventfd_destroy_f* destroy;
};

enum dynfd_flags {
  DFDF_SEND_IMMEDIATELY = (1<<0),
  DFDF_DESTROY_CLEAN    = (1<<1),
  DFDF_DESTROY_ERROR    = (1<<2),
};

struct dynfd {
  const struct dynfd_type* type;
  int fd, flags;
  struct data_queue send_queue;
  struct dynfd* post_process_next;
};

#endif
