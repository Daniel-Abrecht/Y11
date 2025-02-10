#ifndef Y11__MESSAGE_HANDLER_H
#define Y11__MESSAGE_HANDLER_H

#include <-Y11/protocol.h>

struct client_data;

#define C(PARAM, NAME, VALUE) \
  void y11_msg_cb_ ## NAME(struct client_data*const client, uint16_t response_id, y11_msg_ ## NAME ## _t* msg, unsigned payload_size, char payload[restrict payload_size]);
Y11_PROTOCOL_MESSAGES(0,C)
#undef C

void process_messages(struct client_data*const client, const int size, y11_msg_io_buffer_t*restrict const iobuf);

#endif
