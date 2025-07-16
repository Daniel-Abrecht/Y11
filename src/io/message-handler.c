#include <-Y11/S/message-handler.h>
#include <-Y11/S/client.h>
#include <-Y11/S/server.h>
#include <stdio.h>
#include <string.h>

static void process_messages_same(struct y11_s_client*const client, const int size, y11_msg_io_buffer_t*restrict const iobuf){
  int off = 0;
  while(off < size){
    const uint_least16_t opcode   = (*(uint16_t*)&iobuf->data[off  ]);
    const uint_least16_t msg_size = (*(uint16_t*)&iobuf->data[off+2]);
    uint_least16_t payload_size   = 0;
    uint_least16_t response_id    = 0;
    off += 4;
    if(opcode & Y11_MSGOP_HAS_PAYLOAD_OR_RESPONSE_ID){
      payload_size = (*(uint16_t*)&iobuf->data[off  ]);
      response_id  = (*(uint16_t*)&iobuf->data[off+2]);
      off += 4;
    }
    // Note: We may have read up to 7 bytes past size. We keep the last 8 bytes of iobuf unused to meke sure this always works.
    const int noff = off + msg_size*4;
    if(noff > size)
      goto error;
    int header_size = (msg_size*4 - payload_size) & ~3;
    if(header_size < 0)
      goto error;
    switch(opcode & Y11_MSGOP_OPCODE_MASK){
      default: fprintf(stderr, "ignoring unknown message type %04X\n", opcode); break;
#define C(P,X,Y) \
  case Y11_MSG_OP_ ## X: { \
    y11_msg_ ## X ## _t msg = {0}; \
    memcpy(&msg, &iobuf->data[off], header_size); \
    y11_msg_cb_ ## X(client, response_id, &msg, (dpa_u_bo_t){.size=payload_size, .data=&iobuf->data[off]}); \
  } break;
      Y11_PROTOCOL_MESSAGES(0,C)
#undef C
    }
    off = noff;
  }
  return;
error:;
}

static void process_messages_swap(struct y11_s_client* client, int size, y11_msg_io_buffer_t*restrict iobuf){
  int off = 0;
  while(off < size){
    const uint_least16_t opcode   = bswap_16(*(uint16_t*)&iobuf->data[off  ]);
    const uint_least16_t msg_size = bswap_16(*(uint16_t*)&iobuf->data[off+2]);
    uint_least16_t payload_size   = 0;
    uint_least16_t response_id    = 0;
    off += 4;
    if(opcode & Y11_MSGOP_HAS_PAYLOAD_OR_RESPONSE_ID){
      payload_size = bswap_16(*(uint16_t*)&iobuf->data[off  ]);
      response_id  = bswap_16(*(uint16_t*)&iobuf->data[off+2]);
      off += 4;
    }
    const int noff = off + msg_size*4;
    if(noff > size)
      goto error;
    int header_size = (msg_size*4 - payload_size) & ~3;
    if(header_size < 0)
      goto error;
    switch(opcode & Y11_MSGOP_OPCODE_MASK){
      default: fprintf(stderr, "ignoring unknown message type %04X\n", opcode); break;
#define C(P,X,Y) \
  case Y11_MSG_OP_ ## X: { \
    y11_msg_ ## X ## _t msg = {0}; \
    memcpy(&msg, &iobuf->data[off], header_size); \
    y11_msg_swap_endianess__ ## X(&msg); \
    y11_msg_cb_ ## X(client, response_id, &msg, (dpa_u_bo_t){.size=payload_size, .data=&iobuf->data[off]}); \
  } break;
      Y11_PROTOCOL_MESSAGES(0,C)
#undef C
    }
    off = noff;
  }
  return;
error:;
}

void y11_s_process_messages(struct y11_s_client*const client, const int size, y11_msg_io_buffer_t*restrict const iobuf){
  if(dpa_u_unlikely(client->swap_endianess)){
    process_messages_swap(client, size, iobuf);
  }else{
    process_messages_same(client, size, iobuf);
  }
}
