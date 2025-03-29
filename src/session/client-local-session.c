#include <-Y11/S/client-local-session.h>
#include <-Y11/S/client-local.h>
#include <-Y11/S/server.h>
#include <-Y11/S/user.h>
#include <dpa/utils/common.h>
#include <dpa/utils/mem.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

enum y11_endpoint_msg_type_e {
  Y11_ENDPOINT_NEW_CLIENT = 1,
};

static void session_data_handle_message(struct y11_s_session* sd, enum y11_endpoint_msg_type_e op, int fd){
  switch(op){
    case Y11_ENDPOINT_NEW_CLIENT: {
      int domain, type;
      if( getsockopt(fd, SOL_SOCKET, SO_DOMAIN, &domain, (socklen_t[]){sizeof(domain)}) == -1
       || getsockopt(fd, SOL_SOCKET, SO_TYPE, &type, (socklen_t[]){sizeof(type)}) == -1
      ){
        perror("getsockopt");
        break;
      }
      if(domain != PF_UNIX || type != SOCK_SEQPACKET){
        fprintf(stderr, "socket has wrong type!\n");
        break;
      }
      struct ucred cred = {0};
      if(getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &cred, &(socklen_t){sizeof(cred)}) == -1){
        perror("getsockopt SOL_SOCKET SO_PEERCRED");
        break;
      }
      struct y11_s_user* user = y11_s_user_get(cred.uid);
      if(!user){
        fprintf(stderr, "Failed to allocate user %lu\n", (long)cred.uid);
        break;
      }
      fd = fcntl(fd, F_DUPFD_CLOEXEC);
      if(fd == -1){
        perror("dup");
        goto enc_err_user;
      }
      struct y11_s_client_local* lc = dpa_u_copy((struct y11_s_client_local){
        .super = {
          .super = {
            .type = &y11_s_client_local_init_type,
            .fd = fd,
          },
          .user = user,
        },
        .session_user = sd->user,
      });
      if(!lc){
        perror("calloc");
        goto enc_err_dup;
      }
      dpa_s_user_ref(lc->session_user);
      printf("new connection fd:%d (user %lu) from unix socket fd:%d (user %lu)\n", fd, user->uid, sd->super.fd, sd->user->uid);
      if(y11_s_fd_register(&lc->super.super)){
        fprintf(stderr, "failed to add new unix socket client\n");
        goto enc_err_alloc;
      }
      break;
    enc_err_alloc:
      y11_s_user_put(lc->session_user);
      free(lc);
    enc_err_dup:
      close(fd);
    enc_err_user:
      y11_s_user_put(user);
    } break;
  }
}

static void session_ondata(uint32_t events, struct y11_s_fd* dfd){
  (void)events;
  struct y11_s_session* sd = dpa_u_container_of(dfd, struct y11_s_session, super);

  union {
    char   buf[CMSG_SPACE(sizeof(int))];
    struct cmsghdr align;
  } controlMsg;

  struct iovec iov = {0};
  struct msghdr msgh = {0};
  msgh.msg_name = 0;
  msgh.msg_namelen = 0;
  unsigned char data[1];
  iov.iov_base = &data;
  iov.iov_len = sizeof(data);
  msgh.msg_iov = &iov;
  msgh.msg_iovlen = 1;
  msgh.msg_control = controlMsg.buf;
  msgh.msg_controllen = sizeof(controlMsg.buf);
  ssize_t nr = recvmsg(sd->super.fd, &msgh, MSG_CMSG_CLOEXEC);
  struct cmsghdr* cmsgp = CMSG_FIRSTHDR(&msgh);
  if(!nr){
    y11_s_fd_destroy(&sd->super, false);
    return;
  }
  int fd = -1;
  if( cmsgp
   && cmsgp->cmsg_len == CMSG_LEN(sizeof(int))
   && cmsgp->cmsg_level == SOL_SOCKET
   && cmsgp->cmsg_type == SCM_RIGHTS
  ) memcpy(&fd, CMSG_DATA(cmsgp), sizeof(int));
  if(fcntl(fd, F_SETFL, O_NONBLOCK) == -1){
    perror("fcntl");
    goto end;
  }
  session_data_handle_message(sd, data[0], fd);
end:
  close(fd); // Note: If we actually need the fd later, we dup it in session_data_handle_message
}

static void unix_socket_session_destroy(struct y11_s_fd* dfd){
  struct y11_s_session* sd = dpa_u_container_of(dfd, struct y11_s_session, super);
  y11_s_user_put(sd->user);
  free(sd);
}

const struct y11_s_fd_type y11_s_session_type = {
  .ondata = session_ondata,
  .destroy = unix_socket_session_destroy,
};
