#include <-Y11/S/server.h>

int main(){
  server_init();
  while(server_tick());
  return 0;
}
