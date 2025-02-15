#include <-Y11/S/server.h>
#include <-Y11/S/client.h>
#include <-Y11/S/user.h>

void y11_s_client_destroy(struct y11_s_client* client){
  y11_s_user_put(client->user);
  free(client);
}
