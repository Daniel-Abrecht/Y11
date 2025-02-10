#include <-Y11/S/message-handler.h>
#include <-Y11/S/server.h>
#include <stdio.h>

void y11_msg_cb_INIT(struct client_data*const client, uint16_t response_id, y11_msg_INIT_t* msg, unsigned payload_size, char payload[restrict payload_size]){
  (void)msg;
  (void)payload_size;
  (void)payload;
  if(client->super.type != &local_client_init_type){
    fprintf(stderr, "PRotocol error: init message was already sent!\n");
    dynfd_destroy(&client->super, true);
    return;
  }
  client->super.type = &local_client_type;
  puts("INIT");
  Y11_S_SEND_MESSAGE(client,
    (&(y11_msg_INIT_t){
      .version = Y11_VERSION,
    }),
    .response_id = response_id
  );
}
