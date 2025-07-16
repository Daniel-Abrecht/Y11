#ifndef Y11__MESSAGE_HANDLER_H
#define Y11__MESSAGE_HANDLER_H

#include <-Y11/protocol.h>
#include <dpa/utils/bo.h>

struct y11_s_client;

#define C(PARAM, NAME, VALUE) \
  void y11_msg_cb_ ## NAME(struct y11_s_client*const client, uint16_t response_id, y11_msg_ ## NAME ## _t* msg, dpa_u_bo_t payload);
Y11_PROTOCOL_MESSAGES(0,C)
#undef C

void y11_s_process_messages(struct y11_s_client*const client, const int size, y11_msg_io_buffer_t*restrict const iobuf);

#endif
