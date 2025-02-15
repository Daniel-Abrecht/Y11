#include <-Y11/S/message-handler.h>
#include <stdio.h>

void y11_msg_cb_CREATE_ELEMENT(struct y11_s_client*const client, uint16_t response_id, y11_msg_CREATE_ELEMENT_t* msg, unsigned payload_size, char payload[restrict payload_size]){
  (void)client;
  (void)response_id;
  (void)msg;
  (void)payload_size;
  (void)payload;
  puts("CREATE_ELEMENT");
}
