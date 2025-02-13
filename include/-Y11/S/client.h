#ifndef Y11__S_CLIENT_H
#define Y11__S_CLIENT_H

#include <-Y11/S/dynfd.h>
#include <stdbool.h>

struct client_data {
  struct dynfd super;
  bool swap_endianess;
  struct user_data* user;
};

void client_destroy(struct client_data* client);

#endif
