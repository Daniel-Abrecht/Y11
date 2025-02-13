#ifndef Y11__S_SERVER_H
#define Y11__S_SERVER_H

#include <-Y11/S/dynfd.h>
#include <dpa/utils/refcount.h>
#include <stdbool.h>

struct client_data;

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
