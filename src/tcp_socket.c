#include <-Y11/utils.h>
#include <-Y11/S/server.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <errno.h>

#define Y11_PORT 9000 // 1000

static eventfd_handler_f listen_socket_tcp_ondata;
static struct dynfd_type listen_socket_tcp_type = {
  .ondata = listen_socket_tcp_ondata,
};

void init_tcp_socket(void){
  static struct dynfd dfd;
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
  if(add_fd(&dfd)){
    perror("epoll_ctl tcp");
    exit(1);
  }
}

void listen_socket_tcp_ondata(uint32_t events, struct dynfd* dfd){
retry:;
  (void)events;
  const int remote_client_socket = accept4(dfd->fd, NULL, NULL, SOCK_CLOEXEC);
  if(remote_client_socket == -1){
    if(errno == EINTR)
      goto retry;
    if(errno == ECONNABORTED)
      return;
    perror("accept4 tcp");
    exit(1);
  }
  struct remote_client_data* sd = tcopy((struct remote_client_data){
    .super.super.type = &remote_client_type,
    .super.super.fd = remote_client_socket,
  });
  if(!sd){
    perror("calloc");
    goto lsu_error;
  }
  printf("new connection fd:%d from tcp socket (port %d)\n", remote_client_socket, Y11_PORT);
  if(add_fd(&sd->super.super)){
    fprintf(stderr, "failed to add new tcp socket client\n");
    goto lsu_error_malloc;
  }
  return;
lsu_error_malloc:
  free(sd);
lsu_error:
  close(remote_client_socket);
}

