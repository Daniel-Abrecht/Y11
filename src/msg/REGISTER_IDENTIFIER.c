#include <-Y11/S/message-handler.h>
#include <-Y11/S/server.h>
#include <-Y11/S/client.h>
#include <stdio.h>
#include <stdbool.h>

void y11_msg_cb_REGISTER_IDENTIFIER(struct y11_s_client*const client, uint16_t response_id, y11_msg_REGISTER_IDENTIFIER_t* msg, dpa_u_bo_t payload){
  (void)response_id;
  puts("REGISTER_IDENTIFIER");
  if(payload.size < 8){
    fprintf(stderr, "Protocol error: Don't register identifiers < 8.\n");
    y11_s_fd_destroy(&client->super, true);
    return;
  }
  if(!(msg->id.id[0] & 0x08)){
    fprintf(stderr, "Protocol error: first byte of identifier to be registred must have the bit 0b00001000 set.\n");
    y11_s_fd_destroy(&client->super, true);
    return;
  }
  uint64_t cid;
  memcpy(&cid, msg->id.id, 8);
  dpa_u_a_bo_unique_t id = dpa_u_bo_intern(payload);
  if(dpa_u_map_set_if_unset(&client->id_name_map, cid, id)){
    dpa_u_bo_put(id);
    fprintf(stderr, "Protocol error: identifier is already registred.\n");
    y11_s_fd_destroy(&client->super, true);
    return;
  }
}
