#include <hello.h>
#include <am.h>
//#include "../../../klib/klib.h"
#include<klib.h>

int main() {
  char src[10]="hello\n";
  char dst[10];
  strcpy(dst,src);
  printk("%c%d%s",'c',1024,src);
  print(dst);
  assert(1==0);
  //for (int i = 0; i < 10; i ++) {
  //  print("Hello World!\n");
}