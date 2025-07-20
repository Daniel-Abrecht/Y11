
#define Y11_PROTOCOL_MESSAGES(P,X) \
  \
  /*** These commands execute immediately ***/ \
  X(P, INIT,    0x0FFF) /* First message. Determines endianess */ \
  \
  X(P, NOOP,    0x0000) \
  X(P, COMMIT,  0x0001) \
  X(P, DISCARD, 0x0002) \
  \
  X(P, REGISTER_IDENTIFIER,   0x0003) \
  X(P, PUT_IDENTIFIER,        0x0004) \
  X(P, CLIENT_SEND_MESSAGE,   0x0005) \
  X(P, UPDATE_STYLESHEET,     0x0006) \
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

Y11_MESSAGE_COMPONENT(identifier, 4, (
  /**
   * The first nibble of the first byte can be anything.
   * If the second nibble is smaller than 8, it is the length of the inlined itentifier.
   * Otherwise, the identifier needs to be registred using REGISTER_IDENTIFIER.
   */
  uint8_t id[8];
))

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

/**
 * This is the first message sent from the client to the server, and from the server to the client.
 * It contains the latest supported protocol version. Client and server should only respond with messages
 * valid in the protocol version or earlier versions known to their peer. However, they are allowed to append
 * additional fields not yet known to their peer, which must be igonored by them. This significantly simplifies
 * backwards compatibility. Also, keep in mind that leading bytes which are 0 do not have to be transmitted.
 */
Y11_MESSAGE(INIT, 4, (
  uint16_t version;
  char reserved[2];
))

/**
 * This message does nothing. It was originally planned to allow realigning messages of bigger alignment than 4,
 * but no such messages have been implemented, nor is it planed at this point. It's left in just in case.
 */
Y11_MESSAGE(NOOP, 4, (
  char reserved[4]; // This member only exists because structs can't be empty
))

/**
 * Some commands are transactional. They do not take effect until a commit message arrives.
 * If any of them fails, then the commit will fail too, and none of them will be executed.
 * A commit also starts a new transaction.
 */
Y11_MESSAGE(COMMIT, 4, (
  char reserved[4]; // This member only exists because structs can't be empty
))

/**
 * Registers a long identifier, that is, an identifier with a name >= 8 bytes.
 * You mustn't register the same name twice. It may or may not be checked for on the server side,
 * and may or may not fail.
 * You mustn't register the same id twice either, unless it was removed using `PUT_IDENTIFIER` previousely.
 */
Y11_MESSAGE(REGISTER_IDENTIFIER, 4, (
  y11_msg_identifier_t id;
  // identifier name in payload
))

/**
 * Remove a previousely registred long identifier.
 * They currently are not refcounted.
 * Calling this function for an identifier id which is still in use, or does not exist, is an error.
 * The server may or may not check for this.
 */
Y11_MESSAGE(PUT_IDENTIFIER, 4, (
  y11_msg_identifier_t id;
))

/**
 * Discard all commands of the current transaction.
 * They will not be executed. Or at least, it will seam as if.
 */
Y11_MESSAGE(DISCARD, 4, (
  char reserved[4];
))

Y11_MESSAGE_COMPONENT(node, 4, (
  uint32_t id;
))

/**
 * mark a node as used. This node will not be freed by the GC.
 */
Y11_MESSAGE(USE_NODE, 4, (
  y11_msg_node_t node;
))

/**
 * The node is no longer marked as used. The GC can now free it, if none if it's parent nodes are marked as used.
 */
Y11_MESSAGE(PUT_NODE, 4, (
  y11_msg_node_t node;
))

/**
 * This removes a node.
 * This can also be done using the NODE_MOVE_TO command, with a new parent node of 0, and then calling PUT_NODE to allow
 * the GC to remove the node.
 */
Y11_MESSAGE(DESTROY_NODE, 4, (
  y11_msg_node_t node;
))

/**
 * This removes a node and all nodes connected to them.
 */
Y11_MESSAGE(DESTROY_TREE, 4, (
  y11_msg_node_t node;
))

/**
 * This removes a node and all it's child nodes.
 */
Y11_MESSAGE(DESTROY_SUBTREE, 4, (
  y11_msg_node_t node;
))

/**
 * This creates a plug node. Unlike other nodes, they do not have a parent node.
 * Instead, any number of socket nodes can be connected to a plug node.
 * The socket nodes don't necessarely need to be from the same client for this to work.
 * Other clients can't directly access the plug nodes, though.
 * This can be used for templating. For example, it could be a template for a window, a menu, or similar things,
 * or it could just be used to embed something somewhere else.
 */
Y11_MESSAGE(CREATE_PLUG, 4, (
  y11_msg_node_t node;
))

/**
 * Creates a socket node. A socket node can have child nodes.
 * A socket node can also be connected to a plug node, which can belong to a different application.
 * This can be used for templating. For example, maybe an application needs a menu, a window, or something similar
 * to show stuff in.
 */
Y11_MESSAGE(CREATE_SOCKET, 4, (
  y11_msg_node_t node;
))

/**
 * Creates an element node.
 * An element is a node with attributes which can contain other nodes, and can be part of other nodes.
 */
Y11_MESSAGE(CREATE_ELEMENT, 4, (
  y11_msg_node_t node;
  y11_msg_identifier_t tag;
  uint32_t count;
))

/**
 * Creates a text node.
 * Text nodes are used to display text.
 */
Y11_MESSAGE(CREATE_TEXT, 4, (
  y11_msg_node_t node;
  uint32_t count;
))

/**
 * Creates a canvas node.
 * They are meant to display images, video streams, games, and similar content.
 */
Y11_MESSAGE(CREATE_CANVAS, 4, (
  y11_msg_node_t node;
  uint32_t count;
))

/**
 * This command sets an attribute on an element.
 * They can also be set on socket nodes.
 */
Y11_MESSAGE(ELEMENT_SET_ATTRIBUTE, 4, (
  y11_msg_node_t node;
))

/**
 * Moves a node. This changes where a node is in the UI tree.
 */
Y11_MESSAGE(NODE_MOVE_TO, 4, (
  y11_msg_node_t node;
  y11_msg_node_t parent;
  y11_msg_node_t next;
))

Y11_MESSAGE_COMPONENT(plug_id, 4, (
  y11_msg_identifier_t id;
))

/**
 * Attache a plug to a socket.
 */
Y11_MESSAGE(SOCKET_ATTACHE, 4, (
  y11_msg_node_t node;
  y11_msg_plug_id_t plug;
))

/**
 * Send a message to another client.
 * This is one of the ways clients can exchange & make use of plug nodes.
 */
Y11_MESSAGE(CLIENT_SEND_MESSAGE, 4, (
  char reserved[4];
))

/**
 * Modifies style sheet information.
 */
Y11_MESSAGE(UPDATE_STYLESHEET, 4, (
  y11_msg_node_t node;
  // Payload is the modified CSS styles
))
