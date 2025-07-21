#include <stdint.h>
#include <stddef.h>
#include <stdalign.h>

#define Y11_VERSION_MAJOR 0
#define Y11_VERSION_MINOR 1
#define Y11_VERSION ((Y11_VERSION_MAJOR<<8) | Y11_VERSION_MINOR)

// TODO: Should this be removed? It'd probably be better to use the home directory of a system user for running the service
#ifndef Y11_DATADIR
#define Y11_DATADIR "/var/lib/Y11/"
#endif
#ifndef Y11_SOCKET_PATH
#define Y11_SOCKET_PATH Y11_DATADIR "y11.socket"
#endif

#define Y11__UNPACK(...) __VA_ARGS__
#define Y11_MESSAGE(X, A, Y) \
  typedef struct y11_msg_ ## X y11_msg_ ## X ## _t; \
  struct __attribute__((packed,aligned(A))) y11_msg_ ## X { \
    Y11__UNPACK Y \
  };
#define Y11_MESSAGE_COMPONENT(X, A, Y) \
  typedef struct y11_msg_ ## X y11_msg_ ## X ## _t; \
  struct __attribute__((packed,aligned(A))) y11_msg_ ## X { \
    Y11__UNPACK Y \
  };

/**
 * Protocol opcode flags.
 * @{
 */
#define Y11_MSGOP_OPCODE_MASK                 0x0FFFu
#define Y11_MSGOP_TRANSACTIONAL               0x0800u
#define Y11_MSGOP_HAS_PAYLOAD_OR_RESPONSE_ID  0x1000u
/** @} */

#include <Y11/protocol.msg.h>

#undef Y11_MESSAGE
#undef Y11__UNPACK

#define Y11_G_MESSAGE_TYPE_CASE(PARAM, T, V) \
  , y11_msg_ ## T ## _t: Y11_MSG_OP_ ## T \
  , y11_msg_ ## T ## _t*: Y11_MSG_OP_ ## T \
  , const y11_msg_ ## T ## _t*: Y11_MSG_OP_ ## T

#define Y11_G_MESSAGE_TYPE(MSG) _Generic((MSG) Y11_PROTOCOL_MESSAGES(0, Y11_G_MESSAGE_TYPE_CASE))

#define Y11_G_MESSAGE_SWAP_CASE(MSG, T, V) \
  , y11_msg_ ## T ## _t: y11_msg_swap_endianess__##T(&(struct{y11_msg_ ## T ## _t msg;}){DPA_U_G(y11_msg_ ## T ## _t,(MSG))}.msg) \
  , y11_msg_ ## T ## _t*: y11_msg_swap_endianess__##T(&(struct{y11_msg_ ## T ## _t msg;}){DPA_U_G(y11_msg_ ## T ## _t,*(MSG))}.msg) \
  , const y11_msg_ ## T ## _t*: y11_msg_swap_endianess__##T(&(struct{y11_msg_ ## T ## _t msg;}){DPA_U_G(y11_msg_ ## T ## _t,*(MSG))}.msg)

#define Y11_G_MESSAGE_SWAP(MSG) _Generic((MSG) Y11_PROTOCOL_MESSAGES((MSG), Y11_G_MESSAGE_SWAP_CASE))


typedef union y11_msg_any {
#define X(PARAM, NAME, OP) y11_msg_ ## NAME ## _t NAME;
Y11_PROTOCOL_MESSAGES(0, X)
#undef X
} y11_msg_any_t;

typedef struct y11_msg_io_buffer y11_msg_io_buffer_t;
struct y11_msg_io_buffer {
  alignas(max_align_t) char data[4096];
};
