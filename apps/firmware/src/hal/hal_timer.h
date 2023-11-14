#ifndef _HAL_TIMER_H_
#define _HAL_TIMER_H_

typedef unsigned char 	 	byte;
typedef unsigned short	 	word;
typedef unsigned int		dwrd;
typedef unsigned long long 	qwrd;

#define  TMR_REG_BASE    0x15400000

byte TIMER_DELAY_FLAG[2] = {0};

//timer mode config
#define TIMER_ONCE                                	(1 << 1)
#define TIMER_CYCLE                               	(0 << 1)

#define HARDWARE_TIMER_NUM (2)
#define DEFAULT_TIMER_VALUE (0xffffffff)
#define TIMER_BIT_WIDE (0xffffffff)
#define CLK_COUNT_PER_US (2)
#define TIMER0_SYS (0)
#define TIMER1_SYS (1)

#define TIMER_SYS_DELAY_US (124)
#define TIMER_SYS_DELAY_MS (124999)

#define TICKS_PERIOD_US (1000)

#define IRQ_TIMER (1)
#define IRQ_SYS_PRIORITY 0x0

#define TIMER_ENABLE                  (1 << 0)
#define TIMER_DISABLE                 (0 << 0)
#define USER_DEFINED                  (1 << 1)
#define FREE_RUNNING                  (0 << 1)
#define TIMER_INTERRUPT_MASKED        (1 << 2)
#define TIMER_INTERRUPT_UNMASKED      (0 << 2)
#define TIMER_PWM_ENABLE              (1 << 3)
#define TIMER_PWM_DISABLE             (0 << 3)
#define TIMER_0N100PWM_ENABLE         (1 << 4)
#define TIMER_0N100PWM_DISABLE        (0 << 4)

typedef struct timer_regs
{
  volatile dwrd TimerNLoadCount;
  volatile dwrd TimerNCurrentValue;
  volatile dwrd TimerNControlReg;
  volatile dwrd TimerNEOI;
  volatile dwrd TimerNIntStatus;
} timer_regs_t;
#define rd_timer_load_cnt(n)         (*(volatile dwrd *)(TMR_REG_BASE + n*0x14))
#define rd_timer_current_val(n)      (*(volatile dwrd *)(TMR_REG_BASE + 0x4 + n*0x14))
#define rd_timer_control_reg(n)      (*(volatile dwrd *)(TMR_REG_BASE + 0x8 + n*0x14))
#define rd_timer_eol(n)       		 (*(volatile dwrd *)(TMR_REG_BASE + 0xc + n*0x14))
#define rd_timer_int_sts(n)          (*(volatile dwrd *)(TMR_REG_BASE + 0x10 + n*0x14))

#define rd_timers_int_sts            (*(volatile dwrd *)(TMR_REG_BASE + 0xa0))
#define rd_timers_eol                (*(volatile dwrd *)(TMR_REG_BASE + 0xa4))
#define rd_timers_raw_int_sts        (*(volatile dwrd *)(TMR_REG_BASE + 0xa8))
#define rd_timers_comp_ver           (*(volatile dwrd *)(TMR_REG_BASE + 0xac))

typedef void (*call_back_fun_pst_t)(void);
typedef void (*IpIsrFunc) (void);

void hal_hardware_timer_init(void);
void hal_hardware_timer_interrupt_handle(void);
void hal_hardware_timer_delay_ms(dwrd delay_cnt, dwrd timer_id);
void hal_hardware_timer_delay_us(dwrd delay_cnt, dwrd timer_id);
void hal_hardware_timer_system_ticks_cacl(byte whitch_timer);
byte hal_hardware_timer_start(byte whitch_timer, dwrd cycle_us, dwrd once_or_cycle, call_back_fun_pst_t callback_function);
byte hal_hardware_timer_get_count(byte whitch_timer, dwrd *timer_count);
byte hardware_timer_stop(byte whitch_timer);


#endif

