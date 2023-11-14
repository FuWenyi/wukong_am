#include <string.h>
#include <hal_int.h>

extern void hal_uart_print_right_now(const byte *fmt, ...);
#define INTC_INTR_ENTRY_SIZE (sizeof(IpIsrFunc))

/* IntcIntrEntry a segment of memory for interrupt entry table */
IpIsrFunc HalIntcIntrEntryTable[INTC_NUM_OF_CHANNELS];

// void IntcIsrFIQ(void) ALWAYS_INLINE;
void hal_intc_isr_fiq(void)
{
    register IpIsrFunc *p;

    for (p = (IpIsrFunc *)rd_intmask_l; p; p = (IpIsrFunc *)rd_intmask_l) {
        (*p)();
    }
}

// void IntcIsrIRQ(void) ALWAYS_INLINE;
void hal_intc_isr_irq(void)
{
    register IpIsrFunc *p;

    for (p = (IpIsrFunc *)rd_intmask_h; p; p = (IpIsrFunc *)rd_intmask_h) {
        (*p)();
    }
}

void hal_def_irq_isr(void)
{
    hal_uart_print_right_now("irq status 0x%x maskstatus 0x%x finalstatus 0x%x\r\n", rd_sts_l, rd_masksts_l, rd_finsts_l);
}

void hal_intc_unreg_isr(byte bChnlNum)
{
    HalIntcIntrEntryTable[bChnlNum] = NULL;
}

void hal_intc_en_intr(byte bChnlNum)
{
    rd_int_l |= (1 << bChnlNum);
    rd_intmask_l &= (~(1 << bChnlNum));
}

void hal_intc_disable_intr(byte bChnlNum)
{
    rd_int_l &= (~bChnlNum);
    rd_intmask_l |= bChnlNum;
}

void hal_Intc_disable_allintrs(void)
{
    rd_int_l = IRQ_DISABLE;
}

void hal_isr_irq(void)
{
    dwrd current_irq_finalstatus;
    byte irq_status[6] = {rd_finsts_l->ic_irq_finalsts_l.ctrl.uart_status, rd_finsts_l->ic_irq_finalsts_l.ctrl.timer_status, rd_finsts_l->ic_irq_finalsts_l.ctrl.wdt_status,
                             rd_finsts_l->ic_irq_finalsts_l.ctrl.ssi_status, rd_finsts_l->ic_irq_finalsts_l.ctrl.i2c_status, rd_finsts_l->ic_irq_finalsts_l.ctrl.gpio_status};
    byte irq_num = 0;
    current_irq_finalstatus = rd_finsts_l->ic_irq_finalsts_l.whole;

    if (!current_irq_finalstatus)
        return;

    for (irq_num = 0; irq_num < 6; irq_num++) 
    {
        if (irq_status[irq_num])
            (*HalIntcIntrEntryTable[irq_num])();
    }
}

void hal_intc_init(void)
{
    byte ISR_CNT;

    rd_int_l = IRQ_DISABLE;
    rd_intmask_l = IRQ_INIT_MASK;
    rd_irq_lev = IRQ_SYS_PRIORITY;

    for (ISR_CNT = 0; ISR_CNT < INTC_NUM_OF_CHANNELS; ISR_CNT++) 
    {
        rd_irq_pr(ISR_CNT) = IRQ_SYS_PRIORITY;
        HalIntcIntrEntryTable[ISR_CNT] = hal_def_irq_isr;
    }
}

byte hal_intc_irq_isr(byte bChnlNum, byte nPrio, IpIsrFunc isr)
{
    byte irq_num = 0;

    if (bChnlNum >= INTC_NUM_OF_CHANNELS || !isr) 
    {
        return (0);
    }

    if (bChnlNum >= IRQ_GPIO17_Int)
        irq_num = IRQ_GPIO;
    else 
    {
        if (bChnlNum >= IRQ_I2C_Start_det_Int)
            irq_num = IRQ_I2C;
        else
            irq_num = bChnlNum;
    }

    // interrupt entry
    HalIntcIntrEntryTable[irq_num] = isr;
    rd_irq_pr(bChnlNum) = nPrio;
    hal_intc_en_intr(bChnlNum);

    return 1;
}

byte hal_intc_reg_isr(byte bChnlNum, IntcIntrPrio nPrio, IpIsrFunc isr)
{
    if (bChnlNum >= INTC_NUM_OF_CHANNELS || !isr) 
    {
        return (0);
    }

    // interrupt entry
    HalIntcIntrEntryTable[bChnlNum] = isr;

    // source control register
    {
        dwrd r, q;
        dwrd v;

        r = bChnlNum % 4;
        q = bChnlNum / 4;

        // r-m-w
        v = rd_vector_n(q);
        v &= (~(7 << (r * 8)));  /* clear corresponding priority */
        v |= (nPrio << (r * 8)); /* set corresponding priority */
        rd_vector_n(q) = v;
    }

    return (1);
}

#if 0
//void IntcClearIntr(byte bChnlNum) ALWAYS_INLINE;
void IntcClearIntr(byte bChnlNum)
{
    REG_FIQ = 1 << bChnlNum;
    REG_IRQ = 1 << bChnlNum;
    // It will be okay to clear these two registers
    // at the same time.
}

void IntcClearAllIntrs(void)
{
    REG_FIQ = 0xFFFFFFFF;
    REG_IRQ = 0xFFFFFFFF;
}
#endif

