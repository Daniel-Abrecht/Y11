#include <-Y11/S/message-handler.h>
#include <stdio.h>

void y11_msg_cb_ELEMENT_SET_ATTRIBUTE(struct y11_s_client*const client, uint16_t response_id, y11_msg_ELEMENT_SET_ATTRIBUTE_t* msg, dpa_u_bo_t payload){
  (void)client;
  (void)response_id;
  (void)msg;
  (void)payload;
  puts("ELEMENT_SET_ATTRIBUTE");
}
