#ifndef Y11__S_CLIENT_SESSION_DATA_H
#define Y11__S_CLIENT_SESSION_DATA_H

#include <-Y11/S/dynfd.h>

struct session_data {
  struct dynfd super;
  struct user_data* user;
};

#endif
