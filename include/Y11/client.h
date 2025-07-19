#ifndef Y11_CLIENT_H
#define Y11_CLIENT_H

#include <dpa/utils/refcount.h>

typedef struct y11_client y11_client_t;
typedef struct y11_client_endpoint y11_client_endpoint_t;

y11_client_endpoint_t* y11_client_endpoint_new(const char* target);
y11_client_t* y11_client_new(y11_client_endpoint_t* endpoint);

inline void y11_client_ref(y11_client_t* client){
  dpa_u_refcount_increment((dpa_u_refcount_t*)client);
}
inline void y11_client_endpoint_ref(y11_client_endpoint_t* endpoint){
  dpa_u_refcount_increment((dpa_u_refcount_t*)endpoint);
}

void y11_client_put(y11_client_t* client);
void y11_client_endpoint_put(y11_client_endpoint_t* endpoint);

int y11_client_get_fd(y11_client_t* endpoint);

#define Y11_GEN_CALL(X, N) _Generic((X), \
    struct y11_client*: y11_client_ ## N, \
    struct y11_client_endpoint*: y11_client_endpoint_ ## N \
  )

#define y11_ref(X) Y11_GEN_CALL((X), ref)(X)
#define y11_put(X) Y11_GEN_CALL((X), put)(X)

#endif
