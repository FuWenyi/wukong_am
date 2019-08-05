#include <am.h>
#include <riscv32.h>
#include <klib.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void __am_get_cur_as(_Context *c);
void __am_switch(_Context *c);

_Context* __am_irq_handle(_Context *c) {
  __am_get_cur_as(c);

  _Context *next = c;
  if (user_handler) {
    _Event ev = {0};
    switch (c->cause) {
      case 0x80000005: ev.event = _EVENT_IRQ_TIMER; break;
      case 9:
        ev.event = (c->GPR1 == -1) ? _EVENT_YIELD : _EVENT_SYSCALL;
        c->epc += 4;
        break;
      default: ev.event = _EVENT_ERROR; break;
    }

    next = user_handler(ev, c);
    if (next == NULL) {
      next = c;
    }
  }

  __am_switch(next);

  return next;
}

extern void __am_asm_trap(void);

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize exception entry
  asm volatile("csrw stvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  _Context *c = (_Context*)stack.end - 1;

  c->epc = (uintptr_t)entry;
  c->status = 0x000c0100;
  return c;
}

void _yield() {
  asm volatile("li a7, -1; ecall");
}

int _istatus(int enable) {
  return 0;
}
