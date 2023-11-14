#include "hal_wdt.h"

extern byte hal_intc_irq_isr(byte bChnlNum, byte nPrio, IpIsrFunc isr);

void hal_init_watchdog_time(byte mode, dwrd time_period)
{	
    rd_wdt_torr = time_period;
    rd_wdt_cr (mode | WDT_ENABLE);
}

void hal_reset_watchdog(void)
{	
    rd_wdt_crr = RESTART_VALUE;
}

void hal_watchdog_interrupt_handle(void)
{
    if (rd_wdt_stat & 0x1)
        rd_wdt_eol;
}

void hal_wdt_int_mode_init(void)
{
    hal_intc_irq_isr(IRQ_WDT, IRQ_SYS_PRIORITY, hal_watchdog_interrupt_handle);
    hal_init_watchdog_time(INT_MODE, TIME_PERIOD_4MS);
    hal_reset_watchdog();
}

void hal_wdt_rst_mode_init(dwrd time_period)
{
    hal_init_watchdog_time(RST_MODE, time_period);
    hal_reset_watchdog();
}



