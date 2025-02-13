#ifndef Y11__S_CLIENT_LOCAL_H
#define Y11__S_CLIENT_LOCAL_H

#include <-Y11/S/client.h>

struct local_client_data {
  struct client_data super;
  struct user_data* session_user;
};

#endif
