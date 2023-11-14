#include <am.h>
#include <riscv.h>
#include <klib.h>

#define HART_CTRL_RESET_REG_BASE 0x39001000
volatile int core_init_flag[16] = {0};

extern char _heap_start;
extern char _pmem_end;
int main(const char *args);
void __am_init_uartlite(void);
void __am_uartlite_putchar(char ch);

_Area _heap = {
  .start = &_heap_start,
  .end = &_pmem_end,
};

void _putc(char ch) {
  __am_uartlite_putchar(ch);
}

void _halt(int code) {
  __asm__ volatile("mv a0, %0; .word 0x0005006b" : :"r"(code));

  // should not reach here during simulation
  printf("Exit with code = %d\n", code);

  // should not reach here on FPGA
  while (1);
}

unsigned getMhartId() {
  unsigned id = 0;
  __asm volatile ("csrr %[op0], mhartid" : [op0] "=r" (id) : : );
  return id;
}

void _trm_init() {
  //only Core 0 should init uart
  unsigned id = 0;
  __asm volatile ("csrr %[op0], mhartid" : [op0] "=r" (id) : : );
  if (id == 0) {__am_init_uartlite();}

  //if multicore, Core 0 setup Core 1
#ifdef MULTICORE
  #if (MULTICORE>1)
    switch(id) {
      case 0:
        // write hart_ctrl_reset_reg
        *(long*)((long)HART_CTRL_RESET_REG_BASE + 1 * 8) = 0;
        // waiting for hart 1 to update core_init_flag[1]
        while(!core_init_flag[1]){};
        break;
      case 1:
        core_init_flag[id] = 1;
        asm("fence\n");
        break;
      default:
        printf("hart 0x%x has nothing to do\n", id);
        while(1);
    }
  #endif
#endif

  extern const char __am_mainargs;
  int ret = main(&__am_mainargs);
  _halt(ret);
}
