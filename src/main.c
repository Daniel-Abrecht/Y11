#include <-Y11/S/server.h>

int main(){
  y11_s_server_init();
  while(y11_s_server_tick());
  return 0;
}
