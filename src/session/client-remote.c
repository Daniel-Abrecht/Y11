#include <-Y11/S/server.h>
#include <-Y11/S/client-remote.h>

static void remote_client_destroy(struct dynfd* dfd){
  struct remote_client_data* rc = dpa_u_container_of(dfd, struct remote_client_data, super.super);
  client_destroy(&rc->super);
}

static void remote_client_ondata(uint32_t events, struct dynfd* dfd){
  (void)events;
  puts("h_remote_client");
  struct remote_client_data* rc = dpa_u_container_of(dfd, struct remote_client_data, super.super);
  dynfd_destroy(&rc->super.super, false);
}

const struct dynfd_type remote_client_type = {
  .ondata  = remote_client_ondata,
  .destroy = remote_client_destroy,
};

