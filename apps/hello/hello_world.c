#include <am.h>
#include <klib.h>

void success() {
  printf("test passed.\n");
  asm("li a0, 0\n");
  asm(".word 0x0005006b\n");
}

int get_hartid(){
  long hartid = -1;
  asm(
    "csrr a0, mhartid\n"
    "sd a0, %0\n"
    :"=m"(hartid)
    :
    :"a0"
  );
  // printf("get hart id: %lx\n", hartid);
  /*if(hartid < 0){
    // printf("invalid hartid\n");
    assert(0);
  }*/
  return hartid;
}

int main() {
  printf("Hello_world\n");
  return 0;
}
