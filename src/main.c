#include <-Y11/S/server.h>
#include <-Y11/S/user.h>
#include <-Y11/S/message-handler.h>
#include <stdalign.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define IOBUF_SCRATCH_SIZE 8 // See process_messages_same for what this is needed for

static y11_msg_io_buffer_t iobuf[1];

static void client_destroy_sub(struct client_data* client){
  user_put(client->user);
  free(client);
}

static void remote_client_destroy(struct dynfd* dfd){
  struct remote_client_data* rc = dpa_u_container_of(dfd, struct remote_client_data, super.super);
  client_destroy_sub(&rc->super);
}

static void local_client_destroy(struct dynfd* dfd){
  struct local_client_data* lc = dpa_u_container_of(dfd, struct local_client_data, super.super);
  user_put(lc->session_user);
  client_destroy_sub(&lc->super);
}

static void local_client_init_ondata(uint32_t events, struct dynfd* dfd){
  (void)events;
  struct local_client_data* lc = dpa_u_container_of(dfd, struct local_client_data, super.super);
retry:;
  int res = read(lc->super.super.fd, iobuf->data, sizeof(iobuf->data)-IOBUF_SCRATCH_SIZE);
  if(res == 0){
    dynfd_destroy(&lc->super.super, false);
    return;
  }
  if(res < 0){
    if(errno == EINTR) goto retry;
    perror("read");
    dynfd_destroy(&lc->super.super, true);
    return;
  }
  if(res < (int)sizeof(y11_msg_header_short_t)){
    fprintf(stderr, "protocol error, message too small\n");
    dynfd_destroy(&lc->super.super, true);
    return;
  }
  uint16_t opcode = *(uint16_t*)iobuf->data;
  if(opcode == 0xFFFF){
    fprintf(stderr, "protocol error, init message mustn't have all flags set\n");
    dynfd_destroy(&lc->super.super, true);
    return;
  }
  if((opcode & 0xFF0F) == Y11_MSG_OP_INIT_SWAP_ENDIAN){
    lc->super.swap_endianess = true;
  }else if((opcode & Y11_MSGOP_OPCODE_MASK) != Y11_MSG_OP_INIT){
    fprintf(stderr, "protocol error, first message must be an INIT(%04X) message, got %04X\n", Y11_MSG_OP_INIT, opcode);
    dynfd_destroy(&lc->super.super, true);
    return;
  }
  process_messages(&lc->super, res, iobuf);
}

static void remote_client_ondata(uint32_t events, struct dynfd* dfd){
  (void)events;
  puts("h_remote_client");
  struct remote_client_data* rc = dpa_u_container_of(dfd, struct remote_client_data, super.super);
  dynfd_destroy(&rc->super.super, false);
}

static void local_client_ondata(uint32_t events, struct dynfd* dfd){
  (void)events;
retry:;
  struct local_client_data* lc = dpa_u_container_of(dfd, struct local_client_data, super.super);;
  int res = read(lc->super.super.fd, iobuf->data, sizeof(iobuf->data));
  if(res < 0){
    if(errno == EINTR) goto retry;
    // if(errno == EWOULDBLOCK) return;
    perror("read");
    dynfd_destroy(&lc->super.super, true);
    return;
  }
  if(res == 0){
    dynfd_destroy(&lc->super.super, false);
    return;
  }
  process_messages(&lc->super, res, iobuf);
}

const struct dynfd_type remote_client_type = {
  .ondata  = remote_client_ondata,
  .destroy = remote_client_destroy,
};
const struct dynfd_type local_client_type = {
  .ondata  = local_client_ondata,
  .destroy = local_client_destroy,
};
const struct dynfd_type local_client_init_type = {
  .ondata  = local_client_init_ondata,
  .destroy = local_client_destroy,
};

int main(){
  server_init();
  while(server_tick());
  return 0;
}
