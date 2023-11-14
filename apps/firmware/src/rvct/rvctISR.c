#include <global.h>
#include "../hal/hal_int.h"

void __irq FIQ_Handler(void)
{
    //IntcIsrFIQ();
}

void __irq IRQ_Handler(void)
{
    hal_isr_irq();
}

