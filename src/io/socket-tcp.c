#include <-Y11/S/server.h>
#include <-Y11/S/client-remote.h>

#include <dpa/utils/mem.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#define Y11_PORT 9000 // 1000

static dpa_s_fd_handler_f listen_socket_tcp_ondata;
static struct y11_s_fd_type listen_socket_tcp_type = {
  .ondata = listen_socket_tcp_ondata,
};

void y11_s_init_tcp_socket(void){
  static struct y11_s_fd dfd;
  dfd.type = &listen_socket_tcp_type;
  dfd.fd = socket(AF_INET6, SOCK_STREAM | SOCK_CLOEXEC, 0);
  if(dfd.fd < 0){
    perror("socket tcp");
    exit(1);
  }
  if(bind(dfd.fd, (const struct sockaddr*)&(const struct sockaddr_in6){
    .sin6_family = AF_INET6,
    .sin6_port = htons(Y11_PORT)
  }, sizeof(struct sockaddr_in6)) == -1){
    perror("bind tcp");
    exit(1);
  }
  if(listen(dfd.fd, 20) == -1){
    perror("listen tcp");
    exit(1);
  }
  if(y11_s_fd_register(&dfd)){
    perror("epoll_ctl tcp");
    exit(1);
  }
}

static void listen_socket_tcp_ondata(uint32_t events, struct y11_s_fd* dfd){
retry:;
  (void)events;
  const int client_remote_socket = accept4(dfd->fd, NULL, NULL, SOCK_CLOEXEC);
  if(client_remote_socket == -1){
    if(errno == EINTR)
      goto retry;
    if(errno == ECONNABORTED)
      return;
    perror("accept4 tcp");
    exit(1);
  }
  struct y11_s_client_remote* sd = dpa_u_copy((struct y11_s_client_remote){
    .super.super.type = &y11_s_client_remote_type,
    .super.super.fd = client_remote_socket,
  });
  if(!sd){
    perror("calloc");
    goto lsu_error;
  }
  printf("new connection fd:%d from tcp socket (port %d)\n", client_remote_socket, Y11_PORT);
  if(y11_s_fd_register(&sd->super.super)){
    fprintf(stderr, "failed to add new tcp socket client\n");
    goto lsu_error_malloc;
  }
  return;
lsu_error_malloc:
  free(sd);
lsu_error:
  close(client_remote_socket);
}

