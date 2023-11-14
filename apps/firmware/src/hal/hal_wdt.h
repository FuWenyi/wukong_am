#ifndef _HAL_WDT_H
#define _HAL_WDT_H

typedef unsigned char 	 	byte;
typedef unsigned short	 	word;
typedef unsigned int		dwrd;
typedef unsigned long long 	qwrd;

#define  WDT_REG_BASE    0x15500000

typedef struct wdt_regs {
    volatile dwrd WDT_CR;
    volatile dwrd WDT_TORR;
    volatile dwrd WDT_CCVR;
    volatile dwrd WDT_CRR;
    volatile dwrd WDT_STAT;
    volatile dwrd WDT_EOI;
    volatile dwrd WDT_PROT_LEVEL;
} wdt_regs_t;
#define rd_wdt_cr	        (*(volatile dwrd *)(WDT_REG_BASE + 0x0))
#define rd_wdt_torr       	(*(volatile dwrd *)(WDT_REG_BASE + 0x4))
#define rd_wdt_ccvr      	(*(volatile dwrd *)(WDT_REG_BASE + 0x8))
#define rd_wdt_crr       	(*(volatile dwrd *)(WDT_REG_BASE + 0xc))
#define rd_wdt_stat       	(*(volatile dwrd *)(WDT_REG_BASE + 0x10))
#define rd_wdt_eol        	(*(volatile dwrd *)(WDT_REG_BASE + 0x14))
#define rd_wdt_portlev 		(*(volatile dwrd *)(WDT_REG_BASE + 0x1c))


#define WDT_ENABLE (1 << 0)
#define WDT_DISABLE (1 << 1)
#define RST_MODE (0 << 1)
#define INT_MODE (1 << 1)
#define RESTART_VALUE 0x76

#define TIME_PERIOD_4MS 0x33

void hal_wdt_int_mode_init(void);
void hal_init_watchdog_time(byte mode, dwrd time_period);
void hal_reset_watchdog(void);
void hal_wdt_rst_mode_init(dwrd time_period);
#endif
