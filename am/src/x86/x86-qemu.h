#ifndef __AM_X86_H__
#define __AM_X86_H__

#include <am.h>
#include <amdev.h>
#include <amtrace.h>
#include <x86.h>
#include <klib.h> // for debugging

#define MAX_CPU 8

struct boot_info {
  int is_ap;
  void (*entry)();
};
struct cpu_local {
  _AddressSpace *prot;
  SegDesc gdt[NR_SEG];
  TSS tss;
  uint8_t stack[4096];
};
extern volatile uint32_t *__am_lapic;
extern volatile struct boot_info *__am_bootrec;
extern int __am_ncpu;
extern struct cpu_local __am_cpuinfo[MAX_CPU];
#define CPU (&__am_cpuinfo[_cpu()])

#define LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

// apic utils
void __am_lapic_eoi();
void __am_ioapic_init();
void __am_lapic_bootap(unsigned int cpu, uint32_t address);
void __am_ioapic_enable(int irq, int cpu);

// per-cpu x86-specific operations
void __am_bootcpu_init();
void __am_percpu_initirq();
void __am_percpu_initgdt();
void __am_percpu_initlapic();
void __am_percpu_initpg();
void __am_thiscpu_setstk0(uintptr_t ss0, uintptr_t esp0);
void __am_thiscpu_halt() __attribute__((__noreturn__));
void __am_othercpu_halt();

// simple spin locks
#define LOCKDECL(name) \
  void name##_lock(); \
  void name##_unlock();

#define LOCKDEF(name) \
  static volatile intptr_t name##_locked = 0; \
  static int name##_lock_flags[MAX_CPU]; \
  void name##_lock() { \
    name##_lock_flags[_cpu()] = get_efl() & FL_IF; \
    cli(); \
    while (1) { \
      if (0 == _atomic_xchg(&name##_locked, 1)) break; \
      pause(); \
    } \
  } \
  void name##_unlock() { \
    _atomic_xchg(&name##_locked, 0); \
    if (name##_lock_flags[_cpu()]) sti(); \
  }

#define RANGE(st, ed) (_Area) { .start = (void *)st, .end = (void *)ed }
static inline int in_range(void *ptr, _Area area) {
  return area.start <= ptr && ptr < area.end;
}

static inline void puts(const char *s) {
  for (; *s; s++) {
    _putc(*s);
  }
}
#define STRINGIFY(s) #s
#define TOSTRING(s) STRINGIFY(s)
#define panic(s) \
  do { \
    puts("AM Panic: "); puts(s); \
    puts(" @ " __FILE__ ":" TOSTRING(__LINE__) "  \n"); \
    _halt(1); \
  } while(0)

#endif
