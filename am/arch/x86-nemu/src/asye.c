#include <am.h>
#include <x86.h>

static _RegSet* (*H)(_Event, _RegSet*) = NULL;

void irq0();
void vecsys();
void vectrap();
void vecnull();

uintptr_t irq_handle(_RegSet *r) {
  _RegSet *next = r;
  if (H) {
    _Event ev;
    switch (r->irq) {
      case 32: ev.event = _EVENT_IRQ_TIME; break;
      case 0x80: {
        ev.event = _EVENT_SYSCALL;
        intptr_t args[4];
        args[0] = r->eax;
        args[1] = r->edx;
        args[2] = r->ecx;
        args[3] = r->ebx;
        ev.cause = (intptr_t)&args;
        break;
      }
      case 0x81: ev.event = _EVENT_TRAP; break;
      default: ev.event = _EVENT_ERROR; break;
    }

    r->esp = (uintptr_t)r;
    r = H(ev, r);
    if (r != NULL) {
      next = r;
    }
  }

  return next->esp;
}

static GateDesc idt[NR_IRQ];

void _asye_init(_RegSet*(*h)(_Event, _RegSet*)) {
  // initialize IDT
  for (unsigned int i = 0; i < NR_IRQ; i ++) {
    idt[i] = GATE(STS_TG32, KSEL(SEG_KCODE), vecnull, DPL_KERN);
  }

  // --------------------- interrupts --------------------------
  idt[32]   = GATE(STS_IG32, KSEL(SEG_KCODE), irq0,   DPL_KERN);
  // -------------------- system call --------------------------
  idt[0x80] = GATE(STS_TG32, KSEL(SEG_KCODE), vecsys, DPL_USER);
  idt[0x81] = GATE(STS_TG32, KSEL(SEG_KCODE), vectrap, DPL_KERN);
  set_idt(idt, sizeof(idt));
  H = h;
}

_RegSet *_make(_Area stack, void *entry, void *arg) {
  // TODO: pass arg
  stack.end -= 4 * sizeof(int);  // 4 = retaddr + argc + argv + envp
  _RegSet *r = (_RegSet*)stack.end - 1;
  r->esp = (uintptr_t)r;
  r->cs = 0x8;
  r->eip = (uintptr_t)entry;
  r->eflags = 0;
  return r;
}

void _trap() {
  asm volatile("int $0x81");
}

int _istatus(int enable) {
  int ret = (get_efl() & FL_IF) != 0;
  if (enable) {
    sti();
  } else {
    cli();
  }
  return ret;
}
