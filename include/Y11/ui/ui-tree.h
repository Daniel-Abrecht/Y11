#include <stdint.h>
#include <dpa/utils/map.h>

// Note: It's called UI Tree, but since y11ui_plug_t can be attached to multiple y11ui_socket_t, it's hopefully a DAG.
// There is nothing actually preventing cycles though, the client controls the graph and should try not to create cycles.
// The display server will simply keep track of the nodes it has already traversed when traversing them, and skip already traversed nodes.
// This way, cycles won't cause issues for the server, the problem stays a client side problem.
// If the client uses a y11ui_plug_t from another client, it can't always verify that there will be no cycles, though.
// However, the client can't access a y11ui_plug_t object from another client and traverse it's nodes, so such a cycle should not affect it.

typedef uint32_t y11ui_node_index_t;
typedef uint32_t y11ui_client_index_t;

typedef struct y11ui_node y11ui_node_t;
typedef struct y11ui_element y11ui_element_t;
typedef struct y11ui_text y11ui_text_t;
typedef struct y11ui_node_type y11ui_node_type_t;
typedef struct y11ui_socket y11ui_socket_t;
typedef struct y11ui_slot y11ui_slot_t;
typedef struct y11ui_plug y11ui_plug_t;
typedef struct y11ui_node_id y11ui_node_id_t;
typedef struct y11ui_client y11ui_client_t;
typedef struct y11ui_server y11ui_server_t;
typedef struct y11ui_node_with_children y11ui_node_with_children_t;

// Note: we store this in y11ui_node_index_t!
// Currently, we reserve 4 bits for the type, make sure to update Y11UI_NODE_TYPE_BITS if more are needed in the future.
enum y11ui_node_type_e {
  Y11UI_NODE_TYPE_PLUG, // Note: This one is *not* derived from node!
  // The following ones are derived from node
  Y11UI_NODE_TYPE_SOCKET,
  Y11UI_NODE_TYPE_ELEMENT,
  Y11UI_NODE_TYPE_SLOT,
  Y11UI_NODE_TYPE_TEXT,
};

#define Y11UI_NODE_TYPE_BITS 4
#define Y11UI_NODE_INDEX(I) ((I) & ((1u<<(sizeof(y11ui_node_index_t)*CHAR_BIT-Y11UI_NODE_TYPE_BITS))-1u))
#define Y11UI_NODE_TYPE(I)  ((enum y11ui_node_type_e)((I) >> (sizeof(y11ui_node_index_t)*CHAR_BIT-Y11UI_NODE_TYPE_BITS)))

/* The following objects may be in shared memory */

struct y11ui_node_id {
  alignas(uint32_t) struct {
    y11ui_client_index_t client;
    y11ui_node_index_t node;
  };
};

struct y11ui_node {
  y11ui_node_index_t refcount; // Must be the first member
  y11ui_node_index_t next; // Last one points to first entry
  y11ui_node_index_t parent;
  y11ui_node_index_t prev; // First one points to last entry
};

struct y11ui_plug {
  y11ui_node_index_t refcount; // Must be the first member
  // May be attached to any number of sockets of different clients.
  // Different clients do not have direct access to this object, though.
  uint64_t token;
  y11ui_node_index_t child;
};

struct y11ui_slot {
  y11ui_node_t node;
  unsigned first_child_index;
  unsigned child_count;
};

struct y11ui_node_with_children {
  y11ui_node_t node;
  y11ui_node_index_t first_child; // Note: last_child is first_child->prev
  y11ui_node_index_t attribute;
};

struct y11ui_socket {
  y11ui_node_with_children_t super;
  uint64_t token; // A socket may point to a plug of a different client!
  y11ui_node_id_t plug; // This is basically a shadow DOM. A template or overlay if you will. The sockets child nodes may be added somewhere in there, in slot nodes.
};

struct y11ui_node_type {
   int x;
};

struct y11ui_text {
  y11ui_node_t node;
  dpa_u_a_bo_unique_t text;
};

struct y11ui_element {
  y11ui_node_with_children_t super;
  dpa_u_a_bo_unique_t tag;
};

struct y11ui_client {
  unsigned char client_secret[16];
  dpa_u_map_string_t type_map;
  struct {
    size_t count;
    y11ui_plug_t* list;
  } a_plug;
  struct {
    size_t count;
    y11ui_socket_t* list;
  } a_socket;
  struct {
    size_t count;
    y11ui_element_t* list;
  } a_element;
  struct {
    size_t count;
    y11ui_slot_t* list;
  } a_slot;
  struct {
    size_t count;
    y11ui_text_t* list;
  } a_text;
};

extern y11ui_client_t* y11ui_client_self;

// Client stuff

void y11ui_node_remove(y11ui_node_index_t self);
bool y11ui_node_moveto(y11ui_node_index_t self, y11ui_node_index_t parent, y11ui_node_index_t next);
void y11ui_plug_detach(y11ui_node_id_t* plug, y11ui_node_id_t* socket);
bool y11ui_plug_attach(y11ui_node_id_t* plug, y11ui_node_id_t* socket);

// Server stuff

typedef struct y11ui_s_client y11ui_s_client_t;

struct y11ui_s_client {
  struct y11ui_client client;
};

struct y11ui_server {
  struct {
    size_t count;
    y11ui_s_client_t* list;
    dpa_u_map_string_t id_map;
  } client_list;
};
