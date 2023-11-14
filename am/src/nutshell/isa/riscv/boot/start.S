.section entry, "ax"
.globl _start
.type _start, @function

_start:
  mv s0, zero
  la sp, _stack_pointer-0x4000
  # set Core 1 stack
  csrr t0, mhartid
  bnez t0, setup_stack1

to_main:
  jal _trm_init

setup_stack1:
  la sp, _stack_pointer
  j to_main
