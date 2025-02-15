#include <-Y11/utils.h>
#include <-Y11/S/client-local-session.h>
#include <-Y11/S/server.h>
#include <-Y11/S/user.h>
#include <-Y11/protocol.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#define UNIX_SOCKET(...) ((const struct sockaddr*)&(const struct { \
    static_assert(sizeof((struct sockaddr_un){0}.sun_path) >= sizeof(__VA_ARGS__), "UNIX_SOCKET: path is too long: \"" __VA_ARGS__ "\""); \
    struct sockaddr_un un; \
  }){.un={ \
    .sun_family = AF_UNIX, \
    .sun_path = __VA_ARGS__ \
  }}.un), sizeof(struct sockaddr_un)

static dpa_s_fd_handler_f listen_socket_unix_ondata;
static struct y11_s_fd_type listen_socket_unix_type;

void y11_s_init_unix_socket(void){
  static struct y11_s_fd dfd;
  dfd.type = &listen_socket_unix_type;
  dfd.fd = socket(PF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0);
  if(dfd.fd < 0){
    perror("socket unix");
    exit(1);
  }
  unlink(Y11_SOCKET_PATH);
  if(bind(dfd.fd, UNIX_SOCKET(Y11_SOCKET_PATH)) == -1){
    perror("bind unix");
    exit(1);
  }
  if(listen(dfd.fd, 20) == -1){
    perror("listen unix");
    exit(1);
  }
  if(y11_s_fd_register(&dfd)){
    perror("epoll_ctl unix");
    exit(1);
  }
}

static void listen_socket_unix_ondata(uint32_t events, struct y11_s_fd* dfd){
retry:;
  (void)events;
  const int session_socket = accept4(dfd->fd, NULL, NULL, SOCK_CLOEXEC);
  if(session_socket == -1){
    if(errno == EINTR)
      goto retry;
    if(errno == ECONNABORTED)
      return;
    perror("accept4 unix");
    exit(1);
  }
  struct ucred cred = {0};
  if(getsockopt(session_socket, SOL_SOCKET, SO_PEERCRED, &cred, &(socklen_t){sizeof(cred)}) == -1){
    perror("getsockopt SOL_SOCKET SO_PEERCRED");
    goto error;
  }
  struct y11_s_user* user = y11_s_user_get(cred.uid);
  if(!user){
    fprintf(stderr, "Failed to allocate user %lu\n", (long)cred.uid);
    goto error;
  }
  struct y11_s_session* sd = tcopy((struct y11_s_session){
    .super.type = &y11_s_session_type,
    .super.fd = session_socket,
    .user = user,
  });
  if(!sd){
    perror("calloc");
    goto error_user;
  }
  printf("new connection fd:%d from unix socket \"%s\", uid %lu\n", session_socket, Y11_SOCKET_PATH, sd->user->uid);
  if(y11_s_fd_register(&sd->super)){
    fprintf(stderr, "failed to add new unix socket client\n");
    goto error_session;
  }
  return;
error_session:
  free(sd);
error_user:
  y11_s_user_put(user);
error:
  close(session_socket);
}

static struct y11_s_fd_type listen_socket_unix_type = {
  .ondata = listen_socket_unix_ondata
};
