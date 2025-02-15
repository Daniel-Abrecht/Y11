#ifndef Y11__S_USER_H
#define Y11__S_USER_H

#include <dpa/utils/refcount.h>

struct y11_s_user {
  dpa_u_refcount_t refcount;
  long uid;
};

struct y11_s_user* y11_s_user_get(long uid);
void y11_s_user_put(struct y11_s_user* user);

static inline void dpa_s_user_ref(struct y11_s_user* user){
  dpa_u_refcount_ref(user->refcount);
}

#endif
