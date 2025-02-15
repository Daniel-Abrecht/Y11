#ifndef Y11__S_CLIENT_REMOTE_H
#define Y11__S_CLIENT_REMOTE_H

#include <-Y11/S/client.h>

struct y11_s_client_remote {
  struct y11_s_client super;
};

extern const struct y11_s_fd_type y11_s_client_remote_type;

#endif
