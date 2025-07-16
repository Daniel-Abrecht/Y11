#include <-Y11/S/message-handler.h>
#include <-Y11/S/server.h>
#include <-Y11/S/client.h>
#include <-Y11/S/client-local.h>
#include <-Y11/S/client-remote.h>
#include <stdio.h>

void y11_msg_cb_INIT(struct y11_s_client*const client, uint16_t response_id, y11_msg_INIT_t* msg, dpa_u_bo_t payload){
  (void)msg;
  (void)payload;
  if( client->super.type == &y11_s_client_local_type
   // || client->super.type == &y11_s_client_remote_type
  ){
    fprintf(stderr, "Protocol error: init message was already sent!\n");
    y11_s_fd_destroy(&client->super, true);
    return;
  }
  if(client->super.type == &y11_s_client_local_init_type)
    client->super.type = &y11_s_client_local_type;
  puts("INIT");
  Y11_S_SEND_MESSAGE(client,
    (&(y11_msg_INIT_t){
      .version = Y11_VERSION,
    }),
    .response_id = response_id
  );
}
