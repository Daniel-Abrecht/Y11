#include <-Y11/S/server.h>
#include <-Y11/S/client.h>
#include <-Y11/S/user.h>

void y11_s_client_destroy(struct y11_s_client* client){
  for(dpa_u_map_u64_it_fast_t it={0}; dpa_u_map_it_next(&client->id_name_map, &it); )
    dpa_u_bo_put(dpa_u_map_it_get_value(&client->id_name_map, &it).ubo);
  dpa_u_map_clear(&client->id_name_map);
  y11_s_user_put(client->user);
  free(client);
}
