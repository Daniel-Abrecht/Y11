#ifndef Y11__S_USER_H
#define Y11__S_USER_H

#include <dpa/utils/refcount.h>

struct user_data {
  dpa_u_refcount_t refcount;
  long uid;
};

struct user_data* user_get(long uid);
void user_put(struct user_data* user);

static inline void user_ref(struct user_data* user){
  dpa_u_refcount_ref(user->refcount);
}

#endif
