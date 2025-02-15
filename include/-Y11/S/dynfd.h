#ifndef Y11__S_DYNFD_H
#define Y11__S_DYNFD_H

#include <stdint.h>

struct y11_s_fd;

typedef void dpa_s_fd_handler_f(uint32_t events, struct y11_s_fd* dfd);
typedef void dpa_s_fd_destroy_f(struct y11_s_fd* dfd);

enum {
  DQ_INITIAL_CAPACITY     =        64,
  DQ_PAGE_LIMIT           =      4096,
  DQ_MESSAGE_MEMORY_LIMIT = 1024*1024,
};

struct y11_s_data_queue_entry {
  struct y11_s_data_queue_entry* next;
  short size, used;
  char data[];
};

struct y11_s_data_queue {
  struct y11_s_data_queue_entry *first, **last; // Last should be initially set to &first. For simplicity, this is done in y11_s_fd_register for now.
  unsigned size;
  unsigned short offset, count;
};

extern struct y11_s_fd* post_process_list;

struct y11_s_fd_type {
  dpa_s_fd_handler_f* ondata;
  dpa_s_fd_destroy_f* destroy;
};

enum y11_s_fd_flags {
  DFDF_SEND_IMMEDIATELY = (1<<0),
  DFDF_DESTROY_CLEAN    = (1<<1),
  DFDF_DESTROY_ERROR    = (1<<2),
};

struct y11_s_fd {
  const struct y11_s_fd_type* type;
  int fd, flags;
  struct y11_s_data_queue send_queue;
  struct y11_s_fd* post_process_next;
};

#endif
