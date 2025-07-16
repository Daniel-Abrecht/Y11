#include <-Y11/S/message-handler.h>
#include <stdio.h>

void y11_msg_cb_CLIENT_SEND_MESSAGE(struct y11_s_client*const client, uint16_t response_id, y11_msg_CLIENT_SEND_MESSAGE_t* msg, dpa_u_bo_t payload){
  (void)client;
  (void)response_id;
  (void)msg;
  (void)payload;
  puts("CLIENT_SEND_MESSAGE");
}
