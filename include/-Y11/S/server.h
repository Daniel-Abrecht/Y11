#ifndef Y11__S_SERVER_H
#define Y11__S_SERVER_H

#include <-Y11/S/fd.h>
#include <dpa/utils/refcount.h>
#include <stdbool.h>

struct y11_s_client;

int y11_s_fd_register(struct y11_s_fd* dynfd);

void y11_s_server_init(void);
void y11_s_init_unix_socket(void);
void y11_s_init_tcp_socket(void);
bool y11_s_server_tick(void);

void y11_s_fd_destroy(struct y11_s_fd* sd, bool error);

struct iovec;
int y11_s_send(struct y11_s_fd* client, size_t size, int count, struct iovec* io);

typedef struct y11_s_send_msg_args {
  struct y11_s_client* client;
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
    .message = dpa_u_unlikely((CLIENT)->swap_endianess) ? Y11_G_MESSAGE_SWAP(MESSAGE) : (MESSAGE), \
    __VA_ARGS__ \
  })

int y11_s_send_msg(y11_s_send_msg_args_t args);

#endif
