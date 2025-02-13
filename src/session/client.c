#include <-Y11/S/server.h>
#include <-Y11/S/client.h>
#include <-Y11/S/user.h>

void client_destroy(struct client_data* client){
  user_put(client->user);
  free(client);
}
