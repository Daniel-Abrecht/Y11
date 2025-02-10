
#define Y11_PROTOCOL_MESSAGES(P,X) \
  \
  /*** These commands execute immediately ***/ \
  X(P, INIT,    0x0FFF) /* First message. Determines endianess */ \
  \
  X(P, NOOP,    0x0000) \
  X(P, COMMIT,  0x0001) \
  X(P, DISCARD, 0x0002) \
  \
  X(P, CLIENT_SEND_MESSAGE,   0x0003) \
  X(P, UPDATE_STYLESHEET,     0x0004) \
  \
  /*** The following commands may be queued, use OP_COMMIT or OP_DISCARD to commit / discard their changes ***/ \
  X(P, USE_NODE,              0x0000 | Y11_MSGOP_TRANSACTIONAL) \
  X(P, PUT_NODE,              0x0001 | Y11_MSGOP_TRANSACTIONAL) \
  X(P, DESTROY_NODE,          0x0002 | Y11_MSGOP_TRANSACTIONAL) /* Shorthand for OP_NODE_MOVE_TO node -> 0, OP_PUT_NODE node */ \
  X(P, DESTROY_TREE,          0x0003 | Y11_MSGOP_TRANSACTIONAL) /* Same as above, but for all connected nodes too */ \
  X(P, DESTROY_SUBTREE,       0x0004 | Y11_MSGOP_TRANSACTIONAL) /* Same as above, but for all nodes below the specified node */ \
  X(P, CREATE_PLUG,           0x0005 | Y11_MSGOP_TRANSACTIONAL) \
  X(P, CREATE_SOCKET,         0x0006 | Y11_MSGOP_TRANSACTIONAL) \
  X(P, CREATE_ELEMENT,        0x0007 | Y11_MSGOP_TRANSACTIONAL) \
  X(P, CREATE_CANVAS,         0x0008 | Y11_MSGOP_TRANSACTIONAL) \
  X(P, CREATE_TEXT,           0x0009 | Y11_MSGOP_TRANSACTIONAL) \
  X(P, ELEMENT_SET_ATTRIBUTE, 0x000A | Y11_MSGOP_TRANSACTIONAL) \
  X(P, NODE_MOVE_TO,          0x000B | Y11_MSGOP_TRANSACTIONAL) \
  X(P, SOCKET_ATTACHE,        0x000C | Y11_MSGOP_TRANSACTIONAL)

enum y11_msg_opcode {
  Y11_MSG_OP_INIT_SWAP_ENDIAN = 0xFF0F, /* First message. Determines endianess */
#define OP2ENUM(PARAM,NAME,VALUE) Y11_MSG_OP_ ## NAME = VALUE,
  Y11_PROTOCOL_MESSAGES(0, OP2ENUM)
#undef OP2ENUM
};


Y11_MESSAGE_COMPONENT(header_short, 4, (
  uint16_t opcode;
  uint16_t size;
))

Y11_MESSAGE_COMPONENT(header_long, 4, (
  uint16_t opcode;
  uint16_t size;
  uint16_t payload_size;
  uint16_t response_id;
))

Y11_MESSAGE(INIT, 4, (
  uint16_t version;
  char reserved[2];
))

Y11_MESSAGE(NOOP, 4, (
  char reserved[4];
))

Y11_MESSAGE(COMMIT, 4, (
  char reserved[4];
))

Y11_MESSAGE(DISCARD, 4, (
  char reserved[4];
))

Y11_MESSAGE_COMPONENT(node, 4, (
  uint32_t id;
))

Y11_MESSAGE(USE_NODE, 4, (
  y11_msg_node_t node;
))

Y11_MESSAGE(PUT_NODE, 4, (
  y11_msg_node_t node;
))

Y11_MESSAGE(DESTROY_NODE, 4, (
  y11_msg_node_t node;
))

Y11_MESSAGE(DESTROY_TREE, 4, (
  y11_msg_node_t node;
))

Y11_MESSAGE(DESTROY_SUBTREE, 4, (
  y11_msg_node_t node;
))

Y11_MESSAGE(CREATE_PLUG, 4, (
  y11_msg_node_t node;
))

Y11_MESSAGE(CREATE_SOCKET, 4, (
  y11_msg_node_t node;
))

Y11_MESSAGE(CREATE_ELEMENT, 4, (
  y11_msg_node_t node;
))

Y11_MESSAGE(CREATE_TEXT, 4, (
  y11_msg_node_t node;
))

Y11_MESSAGE(CREATE_CANVAS, 4, (
  y11_msg_node_t node;
))

Y11_MESSAGE(ELEMENT_SET_ATTRIBUTE, 4, (
  y11_msg_node_t node;
))

Y11_MESSAGE(NODE_MOVE_TO, 4, (
  y11_msg_node_t node;
  y11_msg_node_t parent;
  y11_msg_node_t next;
))

Y11_MESSAGE_COMPONENT(plug_id, 4, (
  char plug_id[64];
))

Y11_MESSAGE(SOCKET_ATTACHE, 4, (
  y11_msg_node_t node;
  y11_msg_plug_id_t plug;
))

Y11_MESSAGE(CLIENT_SEND_MESSAGE, 4, (
  char reserved[4];
))

Y11_MESSAGE(UPDATE_STYLESHEET, 4, (
  y11_msg_node_t node;
  // followed by css styles
))
