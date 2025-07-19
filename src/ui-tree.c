#include <Y11/ui/ui-tree.h>

y11ui_client_t* y11ui_client_self;

y11ui_node_t* y11ui_get_node(y11ui_node_index_t node){
  const enum y11ui_node_type_e type = Y11UI_NODE_TYPE(node);
  y11ui_node_index_t index = Y11UI_NODE_INDEX(node);
  if(!index--)
    return 0;
  switch(type){
    case Y11UI_NODE_TYPE_PLUG: return 0; // Plug objects aren't regular nodes.
    case Y11UI_NODE_TYPE_SOCKET: return index < y11ui_client_self->a_socket.count ? &y11ui_client_self->a_socket.list[index].super.node : 0;
    case Y11UI_NODE_TYPE_ELEMENT: return index < y11ui_client_self->a_element.count ? &y11ui_client_self->a_element.list[index].super.node : 0;
    case Y11UI_NODE_TYPE_SLOT: return index < y11ui_client_self->a_slot.count ? &y11ui_client_self->a_slot.list[index].node : 0;
    case Y11UI_NODE_TYPE_TEXT: return index < y11ui_client_self->a_text.count ? &y11ui_client_self->a_text.list[index].node : 0;
  }
  return 0;
}

void y11_die(const char* message){
  puts(message);
}

void y11ui_node_unref(y11ui_node_index_t nodeid){
  (void)nodeid;
}

void y11ui_node_ref(y11ui_node_index_t nodeid){
  (void)nodeid;
}

void y11ui_node_remove(y11ui_node_index_t nodeid){
  y11ui_node_t* node = y11ui_get_node(nodeid);
  if(!node)
    return;
  const y11ui_node_index_t parent_type = Y11UI_NODE_TYPE(node->parent);
  y11ui_node_index_t index = Y11UI_NODE_INDEX(node->parent);
  if(!index--)
    return;
  y11ui_node_with_children_t* parent;
  if(parent_type == Y11UI_NODE_TYPE_PLUG){
    if(index < y11ui_client_self->a_plug.count)
      y11ui_client_self->a_plug.list[index].child = 0;
    node->parent = 0;
    y11ui_node_unref(nodeid);
    return;
  }else if(parent_type == Y11UI_NODE_TYPE_SOCKET){
    if(index >= y11ui_client_self->a_socket.count)
      return;
    parent = &y11ui_client_self->a_socket.list[index].super;
  }else if(parent_type == Y11UI_NODE_TYPE_ELEMENT){
    if(index >= y11ui_client_self->a_element.count)
      return;
    parent = &y11ui_client_self->a_element.list[index].super;
  }else return;
  if(node->prev == nodeid){ // first and last node!
    parent->first_child = 0;
  }else{
    if(parent->first_child == nodeid)
      parent->first_child = node->next;
    y11ui_node_t*const next = y11ui_get_node(node->next);
    if(!next) y11_die("The UI Tree is corrupt");
    y11ui_node_t*const prev = y11ui_get_node(node->prev);
    if(!prev) y11_die("The UI Tree is corrupt");
    next->prev = node->prev;
    prev->next = node->next;
  }
  node->prev = nodeid;
  node->next = nodeid;
  node->parent = 0;
  y11ui_node_unref(nodeid);
}

// Note: This function checks for cycles within the subtree.
// Although the server has to be able to handle them, it's nice if clients don't have to worry about them.
bool y11ui_node_moveto(y11ui_node_index_t nodeid, y11ui_node_index_t parentid, y11ui_node_index_t nextid){
  if(parentid == nodeid)
    return false; // This would cause a cycle!
  y11ui_node_t* node = y11ui_get_node(nodeid);
  if(!node)
    return false;
  if(!parentid && nextid)
    return false;
  if(node->next == nextid)
    return node->parent == parentid;
  if(!parentid && !nextid){
    y11ui_node_remove(nodeid);
    return true;
  }
  const y11ui_node_index_t parent_type = Y11UI_NODE_TYPE(parentid);
  if(parent_type == Y11UI_NODE_TYPE_PLUG){
    if(nextid && nextid != nodeid)
      return false;
    y11ui_node_index_t index = Y11UI_NODE_INDEX(parentid)-1;
    if(index < y11ui_client_self->a_plug.count)
      y11ui_client_self->a_plug.list[index].child = nodeid;
    if(!node->parent)
      y11ui_node_ref(nodeid);
    node->parent = parentid;
  }
  if( parent_type != Y11UI_NODE_TYPE_ELEMENT
   && parent_type != Y11UI_NODE_TYPE_SOCKET
  ) return false;
  y11ui_node_t* next = y11ui_get_node(nextid);
  if(nextid && (!next || next->parent != parentid))
    return false;
  y11ui_node_with_children_t* parent = (y11ui_node_with_children_t*)y11ui_get_node(parentid);
  for(y11ui_node_t* it=&parent->node; it; it=y11ui_get_node(it->parent))
    if(it->parent == nodeid)
      return false; // This would cause a cycle!
  if(!nextid){
    nextid = parent->first_child;
    next = y11ui_get_node(nextid);
  }
  y11ui_node_ref(nodeid);
  y11ui_node_remove(nodeid);
  if(!nextid){
    // This is the only node! Next and prev point to themselfes already.
    parent->first_child = nodeid;
  }else{
    node->parent = parentid;
    node->next = nextid;
    node->prev = next->prev;
    next->prev = nodeid;
    y11ui_get_node(node->prev)->next = nodeid;
    if(parent->first_child == node->next)
      parent->first_child = nodeid;
  }
  return true;
}

void y11ui_plug_detach(y11ui_node_id_t* plug, y11ui_node_id_t* socket){
  
}

bool y11ui_plug_attach(y11ui_node_id_t* plug, y11ui_node_id_t* socket){
  
}
