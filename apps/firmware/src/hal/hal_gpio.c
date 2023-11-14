#include <hal_gpio.h>

extern byte hal_intc_irq_isr(byte bChnlNum, byte nPrio, IpIsrFunc isr);

static gpio_t gpio[GPIO_NUM] =
    {
        {0x0, 0xF},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x1, 0x0},
        {0x0, 0x0},
        {0x0, 0x1},
        {0x0, 0x2},
        {0x0, 0x3},
        {0x0, 0x4},
        {0x0, 0x5},
        {0x0, 0x6},
        {0x0, 0x7},
        {0x0, 0x8},
        {0x0, 0x9},
        {0x0, 0xa},
        {0x0, 0xb},
        {0x0, 0xc},
        {0x0, 0xd},
        {0x0, 0xe}};

/*
 * Initializes the GPIO ports.
 *
 * This function initializes the GPIO ports by setting the direction of each
 * port to input, disabling the interrupt source, and setting the output data
 * to low level.
 */
void hal_gpio_init(void)
{
    byte port;  // declare a byte variable named port

    for (port = 0; port < PORTS; port++) 
    {
        rd_gpio_dirction(port) = 0xFFFFFFFF;  // set port direction to input
        rd_gpio_source(port) = 0x0;           // disable interrupt
        rd_gpio_out_data(port) = 0x0;         // set output to low level
    }
}

void hal_gpio_set_output(byte gpio_index)
{
    rd_gpio_source(gpio[gpio_index].port) &= (~(1 << gpio[gpio_index].pin));
    rd_gpio_dirction(gpio[gpio_index].port) |= (1 << gpio[gpio_index].pin);
}

void hal_gpio_set_input(byte gpio_index)
{
    rd_gpio_source(gpio[gpio_index].port) &= (~(1 << gpio[gpio_index].pin));
    rd_gpio_dirction(gpio[gpio_index].port) &= (~(1 << gpio[gpio_index].pin));
}

byte hal_gpio_get_data(byte gpio_index)
{
    hal_gpio_set_input(gpio_index);
    return (rd_gpio_in_data(gpio[gpio_index].port) & (1 << gpio[gpio_index].pin)) ? 1 : 0;
}

void hal_gpio_set_data(byte gpio_index, byte data)
{
    dwrd reg_value;
    hal_gpio_set_output(gpio_index);

    reg_value = rd_gpio_out_data(gpio[gpio_index].port);
    if (data) {
        reg_value |= 1 << gpio[gpio_index].pin;
    }
    else {
        reg_value &= ~(1 << gpio[gpio_index].pin);
    }
    rd_gpio_out_data(gpio[gpio_index].port) = reg_value;
}

void hal_gpio_interrupt_handle(void)
{
    dwrd current_int_status = rd_gpio_int_status;
    byte current_gpio_index = 0;

    while (current_int_status) {
        if (current_int_status & 0x1) {
            if (rd_gpio_int_level & (1 << current_gpio_index))
                rd_gpio_int_clear |= (1 << current_gpio_index);
            else
                rd_gpio_source(0) |= (1 << current_gpio_index);
            // TODO
        }

        current_int_status = (current_int_status >> 1);
        current_gpio_index++;
    }
}

void hal_gpio_int_init(dwrd gpio_index, byte int_level, byte int_polarity)
{
    dwrd gpio_addr = get_gpio_addr(gpio_index);

    hal_intc_irq_isr(gpio_index, IRQ_SYS_PRIORITY, hal_gpio_interrupt_handle);
    // 0:input mode 1:output mode
    rd_gpio_dirction(0) &= (~gpio_addr);
    // 0:Software Mode 1:Hardware Mode
    rd_gpio_source(0) &= (~gpio_addr);
    // 0:level sensitive 1:edge sensitive
    if (int_level)
        rd_gpio_int_level |= gpio_addr;
    else
        rd_gpio_int_level &= (~gpio_addr);
    // 0:Low polarity 1:High polarity
    if (int_polarity)
        rd_gpio_int_polarity |= gpio_addr;
    else
        rd_gpio_int_polarity &= (~gpio_addr);
    rd_gpio_int_mask &= (~gpio_addr);
    rd_gpio_int_enable |= gpio_addr;
}

