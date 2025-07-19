#include <Y11/client.h>
#include <Y11/protocol.h>
#include <unistd.h>

int response_id = 0;

int main(){
  y11_client_endpoint_t* endpoint = y11_client_endpoint_new(0);
  if(!endpoint) return 1;
  y11_client_t* client1 = y11_client_new(endpoint);
  if(!client1) return 1;
  // y11_client_t* client2 = y11_client_new(endpoint);
  // if(!client2) return 1;
  y11_put(endpoint);

  int fd = y11_client_get_fd(client1);
  write(fd, &(struct {
    y11_msg_header_long_t hdr;
    y11_msg_INIT_t msg;
  }){
    .hdr = {
      .opcode = Y11_MSG_OP_INIT | Y11_MSGOP_HAS_PAYLOAD_OR_RESPONSE_ID,
      .size = sizeof(y11_msg_INIT_t) / 4,
      .response_id = ++response_id,
    },
    .msg = {
      .version = Y11_VERSION,
    },
  }, sizeof(y11_msg_header_long_t)+sizeof(y11_msg_INIT_t));

  char buf[256];
  int i = read(fd, buf, sizeof(buf));
  write(1, buf, i);

  y11_put(client1);
  // y11_put(client2);
}
