#ifndef Y11__S_CLIENT_H
#define Y11__S_CLIENT_H

#include <-Y11/S/dynfd.h>
#include <stdbool.h>

struct y11_s_client {
  struct y11_s_fd super;
  bool swap_endianess;
  struct y11_s_user* user;
};

void y11_s_client_destroy(struct y11_s_client* client);

#endif
