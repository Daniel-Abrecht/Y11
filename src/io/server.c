#include <-Y11/S/server.h>
#include <-Y11/S/user.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
// #include <sys/uio.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

static int epollfd;

void y11_s_fd_destroy(struct y11_s_fd* dfd, bool error){
  dfd->flags |= error ? DFDF_DESTROY_CLEAN : DFDF_DESTROY_ERROR;
  if(!dfd->post_process_next && post_process_list != dfd){
    dfd->post_process_next = post_process_list;
    post_process_list = dfd;
  }
}

int y11_s_fd_register(struct y11_s_fd* dynfd){
  if(!dynfd->send_queue.last)
    dynfd->send_queue.last = &dynfd->send_queue.first;
  if(epoll_ctl(
    epollfd, EPOLL_CTL_ADD, dynfd->fd, &(struct epoll_event){
    .events = EPOLLIN,
    .data.ptr = dynfd,
  }) == -1){
    perror("epoll_ctl add");
    return -1;
  }
  return 0;
}

void y11_s_server_init(void){
  umask(0007);
  epollfd = epoll_create1(EPOLL_CLOEXEC);
  if(epollfd == -1){
    perror("epoll_create1");
    exit(1);
  }
  y11_s_init_tcp_socket();
  y11_s_init_unix_socket();
}

struct y11_s_fd* post_process_list;

static void data_queue_clear(struct y11_s_data_queue* queue){
  struct y11_s_data_queue_entry* it = queue->first;
  while(queue->first){
    struct y11_s_data_queue_entry* qe = it;
    it = qe->next;
    free(it);
  }
  queue->first = 0;
  queue->last = &queue->first;
}

static int data_queue_send(int fd, struct y11_s_data_queue* queue){
  unsigned short offset = queue->offset;
  while(queue->first){
    struct y11_s_data_queue_entry* qe = queue->first;
  retry_write:;
    ssize_t res = write(fd, qe->data+offset, qe->used-offset);
    if(res < 0){
      if(errno == EINTR)
        goto retry_write;
      return -1;
    }
    if(res < qe->used-offset){
      offset -= res;
      break;
    }
    offset = 0;
    queue->first = qe->next;
    if(queue->last == &qe->next)
      queue->last = &queue->first;
    free(qe);
  }
  queue->offset = offset;
  return 0;
}

bool y11_s_server_tick(void){
  enum { MAX_EVENTS = 64 };
  struct epoll_event events[MAX_EVENTS];
retry_epoll:;
  int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
  if(nfds == -1){
    if(errno == EINTR)
      goto retry_epoll;
    perror("epoll_wait");
    exit(1);
  }
  for(int i=0; i<nfds; i++){
    const struct epoll_event ev = events[i];
    struct y11_s_fd* dfd = ev.data.ptr;
    if(ev.events & EPOLLOUT && dfd->send_queue.first){
      if(data_queue_send(dfd->fd, &dfd->send_queue) == -1){
        y11_s_fd_destroy(dfd, true);
      }
      if(!dfd->send_queue.first)
      if(epoll_ctl(
        epollfd, EPOLL_CTL_MOD, dfd->fd, &(struct epoll_event){
        .events = EPOLLIN,
        .data.ptr = dfd,
      }) == -1){
        perror("epoll_ctl mod");
        y11_s_fd_destroy(dfd, true);
      }
    }
    if(ev.events & EPOLLIN || ev.events & EPOLLHUP){
      dfd->type->ondata(ev.events, ev.data.ptr);
    }
    if(ev.events & ~(EPOLLOUT|EPOLLIN|EPOLLHUP)){
      // Unhandled event. Maybe EPOLLERR.
      // Whatever it is, we want to drop the client, else poll is gonna keep returning, because this event won't be handled.
      y11_s_fd_destroy(dfd, true);
    }
  }
  struct y11_s_fd*const ppl = post_process_list;
  post_process_list = 0;
  for(struct y11_s_fd *dfd=ppl,*next; dfd; dfd=next){
    next = dfd->post_process_next;
    dfd->post_process_next = 0;
    // Try sending stuff
    if(dfd->flags & DFDF_SEND_IMMEDIATELY && dfd->send_queue.first){
      if(data_queue_send(dfd->fd, &dfd->send_queue) == -1){
        dfd->flags |= DFDF_DESTROY_ERROR;
      }else if(dfd->send_queue.first){
        if(epoll_ctl(
          epollfd, EPOLL_CTL_MOD, dfd->fd, &(struct epoll_event){
          .events = EPOLLOUT,
          .data.ptr = dfd,
        }) == -1){
          perror("epoll_ctl mod");
          dfd->flags |= DFDF_DESTROY_ERROR;
        }
      }
    }
    dfd->flags &= ~DFDF_SEND_IMMEDIATELY;
    if(dfd->flags & DFDF_DESTROY_ERROR || ( dfd->flags & DFDF_DESTROY_CLEAN && (
      !dfd->send_queue.first
    ))){
      if(epoll_ctl(epollfd, EPOLL_CTL_DEL, dfd->fd, 0) == -1){
        perror("epoll_ctl del");
        exit(1);
      }
      if(dfd->flags & DFDF_DESTROY_ERROR)
        if(setsockopt(dfd->fd, SOL_SOCKET, SO_LINGER, &(struct linger){
          .l_onoff = 1,
          .l_linger = 0,
        }, sizeof(struct linger)) < 0)
          perror("setsockopt");
      close(dfd->fd);
      printf("connection fd:%d closed\n", dfd->fd);
      data_queue_clear(&dfd->send_queue);
      dfd->type->destroy(dfd);
      dfd = 0;
    }
  }
  return true;
}
