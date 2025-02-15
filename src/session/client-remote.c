#include <-Y11/S/server.h>
#include <-Y11/S/client-remote.h>

static void client_remote_destroy(struct y11_s_fd* dfd){
  struct y11_s_client_remote* rc = dpa_u_container_of(dfd, struct y11_s_client_remote, super.super);
  y11_s_client_destroy(&rc->super);
}

static void client_remote_ondata(uint32_t events, struct y11_s_fd* dfd){
  (void)events;
  puts("h_client_remote");
  struct y11_s_client_remote* rc = dpa_u_container_of(dfd, struct y11_s_client_remote, super.super);
  y11_s_fd_destroy(&rc->super.super, false);
}

const struct y11_s_fd_type y11_s_client_remote_type = {
  .ondata  = client_remote_ondata,
  .destroy = client_remote_destroy,
};

