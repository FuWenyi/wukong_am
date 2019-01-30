#include <am.h>
#include <riscv32.h>
#include <klib.h>

static _Context* (*user_handler)(_Event, _Context*) = NULL;

void get_cur_as(_Context *c);
void _switch(_Context *c);

_Context* irq_handle(_Context *c) {
//  get_cur_as(c);

  _Context *next = c;
  if (user_handler) {
    _Event ev = {0};
    switch (c->cause) {
      //case 0: ev.event = _EVENT_IRQ_TIMER; break;
      case 11:
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

//  _switch(next);

  return next;
}

extern void asm_trap(void);

int _cte_init(_Context*(*handler)(_Event, _Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(asm_trap));

  // register event handler
  user_handler = handler;

  return 0;
}

_Context *_kcontext(_Area stack, void (*entry)(void *), void *arg) {
  _Context *c = (_Context*)stack.end - 1;

  c->epc = (uintptr_t)entry;
  return c;
}

void _yield() {
  asm volatile("li a0, -1; ecall");
}

int _istatus(int enable) {
  return 0;
}
