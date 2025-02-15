#ifndef Y11__S_CLIENT_SESSION_DATA_H
#define Y11__S_CLIENT_SESSION_DATA_H

#include <-Y11/S/fd.h>

struct y11_s_session {
  struct y11_s_fd super;
  struct y11_s_user* user;
};

extern const struct y11_s_fd_type y11_s_session_type;

#endif
