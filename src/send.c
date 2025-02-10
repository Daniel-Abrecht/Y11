#include <sys/uio.h>
#include <-Y11/S/server.h>
#include <-Y11/protocol.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>


int y11_s_send_msg(y11_s_send_msg_args_t args){
  // TODO: swap message fields if needed
  (void)args;
  if(args.message_size % 4)
    return -1; // Size must be a multiple of 4
  args.message_size /= 4;
  uint16_t opcode = args.type;
  if(args.response_id || args.payload_size)
    opcode |= Y11_MSGOP_HAS_PAYLOAD_OR_RESPONSE_ID;
  // null bytes in message headers don't need to be transmitted
  while(args.message_size && ((uint32_t*)args.message)[args.message_size-1])
    args.message_size--;
  y11_msg_header_long_t header = {
    .opcode = bswap_16(opcode),
    .size   = bswap_16(args.message_size),
    .payload_size = bswap_16(args.payload_size),
    .response_id  = bswap_16(args.response_id ),
  };
  struct iovec parts[] = {
    {
      .iov_base = &header,
      .iov_len  = opcode & Y11_MSGOP_HAS_PAYLOAD_OR_RESPONSE_ID ? 8 : 4,
    },{
      .iov_base = args.message,
      .iov_len  = args.message_size*4,
    },{
      .iov_base = args.payload,
      .iov_len  = args.payload_size,
    },{
      .iov_base = (char[4]){0}, // Padding. Messages must be multiple of 4 bytes
      .iov_len  = -args.payload_size & 3,
    }
  };
  size_t size = parts[0].iov_len + parts[1].iov_len + parts[2].iov_len + parts[3].iov_len;
  return y11_s_send(&args.client->super, size, 4, parts);
}

int y11_s_send(struct dynfd* dfd, size_t size, int count, struct iovec* io){
  if(!size)
    return 0;
  if(size > DQ_PAGE_LIMIT)
    return -1;
  data_queue_t*restrict queue = &dfd->send_queue;
  data_queue_entry_t*restrict qe = *queue->last;
  const bool send_already_pending = qe;
  if(qe && qe->used+size <= DQ_PAGE_LIMIT){
    short new_used = qe->used + size;
    short queue_size = qe->size;
    if(new_used > queue_size){
      while(queue_size < new_used)
        queue_size *= 2;
      qe = realloc(qe, sizeof(data_queue_entry_t) + queue_size);
      if(!qe)
        return -1;
      *queue->last = qe; // Initially, this points to queue->first. Later, it points to the entry->next field of the previous entry.
      qe->size = queue_size;
    }
    size_t off = 0;
    for(int i=0; i<count; i++){
      memcpy(qe->data+qe->used+off, io[i].iov_base, io[i].iov_len);
      off += io[i].iov_len;
    }
    qe->used = new_used;
  }else{
    short entry_size = size;
    entry_size--;
    entry_size |= entry_size >> 1;
    entry_size |= entry_size >> 2;
    entry_size |= entry_size >> 4;
    entry_size |= entry_size >> 8;
    entry_size++;
    if(entry_size < DQ_INITIAL_CAPACITY)
      entry_size = DQ_INITIAL_CAPACITY;
    qe = calloc(1, sizeof(data_queue_entry_t) + entry_size);
    if(!qe)
      return -1;
    if(*queue->last)
      queue->last = &(*queue->last)->next;
    qe->next = 0;
    qe->size = entry_size;
    qe->used = size;
    size_t off = 0;
    for(int i=0; i<count; i++){
      memcpy(qe->data+off, io[i].iov_base, io[i].iov_len);
      off += io[i].iov_len;
    }
    *queue->last = qe; // Initially, this points to queue->first. Later, it points to the entry->next field of the previous entry.
    queue->count++;
    queue->size += size;
  }
  if(!send_already_pending && !dfd->post_process_next && post_process_list != dfd){
    dfd->flags |= DFDF_SEND_IMMEDIATELY;
    dfd->post_process_next = post_process_list;
    post_process_list = dfd;
  }
  return 0;
}
