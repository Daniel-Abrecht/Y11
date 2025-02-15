#ifndef Y11__S_CLIENT_LOCAL_H
#define Y11__S_CLIENT_LOCAL_H

#include <-Y11/S/client.h>

struct y11_s_client_local {
  struct y11_s_client super;
  struct y11_s_user* session_user;
};

extern const struct y11_s_fd_type y11_s_client_local_type;
extern const struct y11_s_fd_type y11_s_client_local_init_type;

#endif
