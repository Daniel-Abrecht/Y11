#include <-Y11/S/user.h>
#include <dpa/utils/map.h>
#include <stdlib.h>

static dpa_u_map_lu_t user_map;

struct user_data* user_get(long uid){
  dpa_u_optional_pointer_t res = dpa_u_map_get(&user_map, uid);
  if(res.present){
    struct user_data* user = res.value;
    dpa_u_refcount_ref(&user->refcount);
    return user;
  }
  struct user_data* user = calloc(1, sizeof(*user));
  user->uid = uid;
  dpa_u_refcount_ref(&user->refcount);
  dpa_u_map_set(&user_map, uid, user);
  puts("user allocated");
  return user;
}

void user_put(struct user_data* user){
  if(!user) return;
  if(dpa_u_refcount_decrement(&user->refcount))
    return;
  dpa_u_map_remove(&user_map, user->uid);
  free(user);
  puts("user freed");
}
