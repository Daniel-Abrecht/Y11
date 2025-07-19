#include <dpa/utils/refcount.h>
#include <Y11/client.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

enum Y11_client_endpoint_type_e {
  Y11_CT_UNIX_SOCKET,
  Y11_CT_ADDRESS,
};

enum endpoint_msg_type_e {
  ENDPOINT_NEW_CLIENT = 1,
};

struct y11_client_endpoint {
  dpa_u_refcount_t refcount;
  enum Y11_client_endpoint_type_e type;
  union {
    struct {
      bool shared;
      int fd;
    };
    char* address;
  };
};

struct y11_client {
  dpa_u_refcount_t refcount;
  y11_client_endpoint_t* endpoint;
  int fd;
};

y11_client_endpoint_t* y11_client_endpoint_new(const char* target){
  y11_client_endpoint_t*restrict endpoint = calloc(1, sizeof(endpoint));
  if(!endpoint){
    perror("calloc");
    return 0;
  }
  if(!target)
    target = getenv("Y11_SERVER");
  if(!target)
    target = "/var/lib/Y11/y11.socket";
  endpoint->fd = -1;
  if(target[0] == '/'){
    const int fd = open(target, O_PATH|O_CLOEXEC);
    if(fd == -1){
      perror("open");
      goto error;
    }
    endpoint->fd = socket(PF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
    if(endpoint->fd == -1){
      perror("socket");
      close(fd);
      goto error;
    }
    struct sockaddr_un sockaddr_un = {0};
    sockaddr_un.sun_family = AF_UNIX;
    snprintf(sockaddr_un.sun_path, sizeof(sockaddr_un.sun_path), "/proc/self/fd/%d", fd);
    if(connect(endpoint->fd, (struct sockaddr*)&sockaddr_un, sizeof(struct sockaddr_un)) == -1){
      perror("connect");
      close(fd);
      goto error;
    }
    close(fd);
  }else if(!strncmp("fd:",target,3)){
    endpoint->shared = true;
    endpoint->fd = atoi(target+3);
  }else if(!strncmp("remote:",target,7)){
    endpoint->type = Y11_CT_ADDRESS;
    endpoint->address = strdup(target+7);
  }
  if(endpoint->fd < 0){
    fprintf(stderr, "Couldn't parse Y11_SERVER option");
    goto error;
  }
  int domain, type;
  if( getsockopt(endpoint->fd, SOL_SOCKET, SO_DOMAIN, &domain, (socklen_t[]){sizeof(domain)}) == -1
   || getsockopt(endpoint->fd, SOL_SOCKET, SO_TYPE, &type, (socklen_t[]){sizeof(type)}) == -1
  ){
    perror("getsockopt");
    goto error;
  }
  if(domain != PF_UNIX || type != SOCK_SEQPACKET){
    perror("socket has wrong type!");
    goto error;
  }
  dpa_u_refcount_increment(&endpoint->refcount);
  return endpoint;
error:
  if(!endpoint->shared)
    close(endpoint->fd);
  free(endpoint);
  return 0;
}

y11_client_t* y11_client_new(y11_client_endpoint_t* endpoint){
  if(!endpoint) return 0;
  y11_client_t*restrict client = calloc(1, sizeof(client));
  if(!client){
    perror("calloc");
    return 0;
  }
  int fds[2];
  if(socketpair(PF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, fds) == -1)
    goto error;
  {
    struct iovec iov = {
      .iov_base = (unsigned char[]){ENDPOINT_NEW_CLIENT}, // Must send at least one byte
      .iov_len = 1,
    };
    union {
      char buf[CMSG_SPACE(sizeof(int))];
      struct cmsghdr align;
    } u;
    struct msghdr msg = {
      .msg_iov = &iov,
      .msg_iovlen = 1,
      .msg_control = u.buf,
      .msg_controllen = sizeof(u.buf)
    };
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    *cmsg = (struct cmsghdr){
      .cmsg_level = SOL_SOCKET,
      .cmsg_type = SCM_RIGHTS,
      .cmsg_len = CMSG_LEN(sizeof(int))
    };
    memcpy(CMSG_DATA(cmsg), &fds[1], sizeof(int));
    sendmsg(endpoint->fd, &msg, 0);
    close(fds[1]);
  }

  dpa_u_refcount_increment(&endpoint->refcount);
  client->endpoint = endpoint;
  client->fd = fds[0];
  dpa_u_refcount_increment(client->refcount);
  return client;
error:
  free(client);
  return 0;
}

extern void y11_client_ref(y11_client_t* client);
extern void y11_client_endpoint_ref(y11_client_endpoint_t* client);

void y11_client_put(y11_client_t* client){
  if(dpa_u_refcount_decrement(&client->refcount))
    return;
  y11_client_endpoint_put(client->endpoint);
  free(client);
}

void y11_client_endpoint_put(y11_client_endpoint_t* endpoint){
  if(dpa_u_refcount_decrement(&endpoint->refcount))
    return;
  if(!endpoint->shared)
    close(endpoint->fd);
  free(endpoint);
}

int y11_client_get_fd(y11_client_t* client){
  return client->fd;
}
