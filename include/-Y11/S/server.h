#ifndef Y11__S_SERVER_H
#define Y11__S_SERVER_H

#include <assert.h>
#include <stdint.h>
#include <stdbool.h>
#include <dpa/utils/refcount.h>

#define UNIX_SOCKET(...) ((const struct sockaddr*)&(const struct { \
    static_assert(sizeof((struct sockaddr_un){0}.sun_path) >= sizeof(__VA_ARGS__), "UNIX_SOCKET: path is too long: \"" __VA_ARGS__ "\""); \
    struct sockaddr_un un; \
  }){.un={ \
    .sun_family = AF_UNIX, \
    .sun_path = __VA_ARGS__ \
  }}.un), sizeof(struct sockaddr_un)

enum {
  DQ_INITIAL_CAPACITY     =        64,
  DQ_PAGE_LIMIT           =      4096,
  DQ_MESSAGE_MEMORY_LIMIT = 1024*1024,
};

typedef struct data_queue_entry data_queue_entry_t;
typedef struct data_queue data_queue_t;

struct dynfd;
typedef void eventfd_handler_f(uint32_t events, struct dynfd* dfd);
typedef void eventfd_destroy_f(struct dynfd* dfd);

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

struct session_data {
  struct dynfd super;
  struct user_data* user;
};

enum client_data_type {
  T_LOCAL_CLIENT_DATA,
  T_REMOTE_CLIENT_DATA
};

struct client_data {
  struct dynfd super;
  unsigned char type;
  bool swap_endianess;
  struct user_data* user;
};

struct local_client_data {
  struct client_data super;
  struct user_data* session_user;
};

struct remote_client_data {
  struct client_data super;
};

extern const struct dynfd_type session_data_type;
extern const struct dynfd_type remote_client_type;
extern const struct dynfd_type local_client_type;
extern const struct dynfd_type local_client_init_type;

int add_fd(struct dynfd* dynfd);
void remove_fd(int fd);

void server_init(void);
void init_unix_socket(void);
void init_tcp_socket(void);
bool server_tick(void);

void dynfd_destroy(struct dynfd* sd, bool error);

struct iovec;
int y11_s_send(struct dynfd* client, size_t size, int count, struct iovec* io);

typedef struct y11_s_send_msg_args {
  struct client_data* client;
  // enum y11_msg_opcode type;
  int type;
  unsigned message_size;
  void* message;
  unsigned payload_size;
  void* payload;
  int response_id;
  int placeholder;
} y11_s_send_msg_args_t;

#define Y11_S_SEND_MESSAGE(...) Y11_S_SEND_MESSAGE_S(__VA_ARGS__, 0)
#define Y11_S_SEND_MESSAGE_S(CLIENT, MESSAGE, ...) \
  y11_s_send_msg((y11_s_send_msg_args_t){ \
    .client = (CLIENT), \
    .type = Y11_G_MESSAGE_TYPE((MESSAGE)), \
    .message_size = sizeof(*(MESSAGE)), \
    .message = (CLIENT)->swap_endianess ? Y11_G_MESSAGE_SWAP(MESSAGE) : (MESSAGE), \
    __VA_ARGS__ \
  })

int y11_s_send_msg(y11_s_send_msg_args_t args);

#endif
