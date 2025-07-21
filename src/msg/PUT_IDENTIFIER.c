#include <-Y11/S/message-handler.h>
#include <-Y11/S/client.h>
#include <stdio.h>

void y11_msg_cb_PUT_IDENTIFIER(struct y11_s_client*const client, uint16_t response_id, y11_msg_PUT_IDENTIFIER_t* msg, dpa_u_bo_t payload){
  (void)response_id;
  (void)payload;
  uint64_t cid;
  memcpy(&cid, msg->id.id, 8);
  dpa_u_optional_t old_value = dpa_u_map_get_and_remove(&client->id_name_map, cid);
  if(old_value.present)
    dpa_u_bo_put(old_value.value.ubo);
}
