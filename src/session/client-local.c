#include <-Y11/S/message-handler.h>
#include <-Y11/S/client-local.h>
#include <-Y11/S/server.h>
#include <-Y11/S/user.h>
#include <unistd.h>
#include <errno.h>

#define IOBUF_SCRATCH_SIZE 8 // See process_messages_same for what this is needed for

static y11_msg_io_buffer_t iobuf[1];

static void client_local_destroy(struct y11_s_fd* dfd){
  struct y11_s_client_local* lc = dpa_u_container_of(dfd, struct y11_s_client_local, super.super);
  y11_s_user_put(lc->session_user);
  y11_s_client_destroy(&lc->super);
}

static void client_local_init_ondata(uint32_t events, struct y11_s_fd* dfd){
  (void)events;
  struct y11_s_client_local* lc = dpa_u_container_of(dfd, struct y11_s_client_local, super.super);
retry:;
  int res = read(lc->super.super.fd, iobuf->data, sizeof(iobuf->data)-IOBUF_SCRATCH_SIZE);
  if(res == 0){
    y11_s_fd_destroy(&lc->super.super, false);
    return;
  }
  if(res < 0){
    if(errno == EINTR) goto retry;
    perror("read");
    y11_s_fd_destroy(&lc->super.super, true);
    return;
  }
  if(res < (int)sizeof(y11_msg_header_short_t)){
    fprintf(stderr, "protocol error, message too small\n");
    y11_s_fd_destroy(&lc->super.super, true);
    return;
  }
  uint16_t opcode = *(uint16_t*)iobuf->data;
  if(opcode == 0xFFFF){
    fprintf(stderr, "protocol error, init message mustn't have all flags set\n");
    y11_s_fd_destroy(&lc->super.super, true);
    return;
  }
  if((opcode & 0xFF0F) == Y11_MSG_OP_INIT_SWAP_ENDIAN){
    lc->super.swap_endianess = true;
  }else if((opcode & Y11_MSGOP_OPCODE_MASK) != Y11_MSG_OP_INIT){
    fprintf(stderr, "protocol error, first message must be an INIT(%04X) message, got %04X\n", Y11_MSG_OP_INIT, opcode);
    y11_s_fd_destroy(&lc->super.super, true);
    return;
  }
  y11_s_process_messages(&lc->super, res, iobuf);
}

static void client_local_ondata(uint32_t events, struct y11_s_fd* dfd){
  (void)events;
retry:;
  struct y11_s_client_local* lc = dpa_u_container_of(dfd, struct y11_s_client_local, super.super);;
  int res = read(lc->super.super.fd, iobuf->data, sizeof(iobuf->data));
  if(res < 0){
    if(errno == EINTR) goto retry;
    // if(errno == EWOULDBLOCK) return;
    perror("read");
    y11_s_fd_destroy(&lc->super.super, true);
    return;
  }
  if(res == 0){
    y11_s_fd_destroy(&lc->super.super, false);
    return;
  }
  y11_s_process_messages(&lc->super, res, iobuf);
}

const struct y11_s_fd_type y11_s_client_local_type = {
  .ondata  = client_local_ondata,
  .destroy = client_local_destroy,
};
const struct y11_s_fd_type y11_s_client_local_init_type = {
  .ondata  = client_local_init_ondata,
  .destroy = client_local_destroy,
};
