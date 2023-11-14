1.fuwenyi changes riscv64 cross-compiler from riscv64-linux-gnu- to riscv64-unknown-elf-
2.for #define MULTICORE 2 in am/include/riscv.h

为了搞明白反汇编和c语言的对应关系，添加-g -Wa,-alh选项到Makefile.compile: CFLAGS