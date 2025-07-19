#include <Y11/ui/ui-tree.h>

int main(){
  printf("plug:\t%zd\n", sizeof(y11ui_plug_t));
  printf("socket:\t%zd\n", sizeof(y11ui_socket_t));
  printf("slot:\t%zd\n", sizeof(y11ui_slot_t));
  printf("element:\t%zd\n", sizeof(y11ui_element_t));
  printf("text:\t%zd\n", sizeof(y11ui_text_t));
}
