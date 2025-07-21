#ifndef Y11__S_CLIENT_H
#define Y11__S_CLIENT_H

#include <-Y11/S/fd.h>
#include <dpa/utils/map.h>
#include <dpa/utils/set.h>
#include <stdbool.h>

struct y11_s_client {
  struct y11_s_fd super;
  bool swap_endianess;
  struct y11_s_user* user;
  dpa_u_map_u64_t id_name_map;
  dpa_u_set_string_t id_set;
};

void y11_s_client_destroy(struct y11_s_client* client);

#endif
