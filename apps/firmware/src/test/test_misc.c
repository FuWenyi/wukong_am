#include <test_misc.h>
#include "../hal/hal_spi.h"
#include "../hal/hal_wdt.h"
#include "../hal/hal_timer.h"
#include "../hal/hal_int.h"
#include "../hal/hal_gpio.h"
#include "../hal/hal_i2c.h"

extern void hal_uart_print_right_now(const byte *fmt, ...);

dwrd hal_spi_test(void)
{
    dwrd test_addr = 0x410000;
    dwrd test_len = 0x4000;
    dwrd test_index = 0;

    hal_uart_print_right_now("[spi test] --> start\r\n");

    for (test_index = 0; test_index < test_len; test_index += 4)
    {
    	(*(volatile dwrd *)(test_addr + test_index)) = (test_addr + test_index);
    }

    hal_spi_init();
    hal_spi_chip_erase();
    hal_spi_write(test_addr, 0x0, test_len);
    hal_spi_read(test_addr + test_len, 0x0, test_len);

    for (test_index = 0; test_index < test_len; test_index += 4)
    {
        if ((*(volatile dwrd *)(test_addr + test_len + test_index)) != (*(volatile dwrd *)(test_addr + test_index)))
        {
            hal_uart_print_right_now("[spi test] <-- fail\r\n");
            return 1;
        }
    }

    hal_spi_chip_erase();
    hal_spi_read(test_addr + test_len, 0x0, test_len);
    for (test_index = 0; test_index < test_len; test_index += 4)
    {
        if ((*(volatile dwrd *)(test_addr + test_len + test_index)) != 0xFFFFFFFF)
        {
            hal_uart_print_right_now("[spi test] <-- fail\r\n");
            return 1;
        }
    }

    hal_uart_print_right_now("[spi test] <-- pass\r\n");
    return 0;
}


//gpio test
static byte gpio_test_flag = 0;

void hal_gpio_interrupt_test_handle(void)
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
    gpio_test_flag = 1;
}

void hal_gpio_test_init(dwrd gpio_index, byte int_level, byte int_polarity)
{
    dwrd gpio_addr = get_gpio_addr(gpio_index);

    hal_intc_irq_isr(gpio_index, IRQ_SYS_PRIORITY, hal_gpio_interrupt_test_handle);

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
    rd_gpio_int_enable |= gpio_addr;
    rd_gpio_int_mask &= (~gpio_addr);
}

dwrd hal_gpio_test(void)
{
    dwrd delay_cnt = 0;
    hal_uart_print_right_now("[gpio test] --> start\r\n");

    hal_gpio_test_init(IRQ_GPIO17_Int, LEVEL_SENSITIVE, HIGH_POLARITY);

    while (!gpio_test_flag) {
        delay_cnt++;
        if (delay_cnt > 1000) {
            hal_uart_print_right_now("[gpio test] <-- fail\r\n");
            hal_intc_irq_isr(IRQ_GPIO17_Int, IRQ_SYS_PRIORITY, hal_gpio_interrupt_handle);
            return 1;
        }
    }
    hal_intc_irq_isr(IRQ_GPIO17_Int, IRQ_SYS_PRIORITY, hal_gpio_interrupt_handle);
    hal_uart_print_right_now("[gpio test] <-- pass\r\n");

    return 0;
}

dwrd hal_i2c_test(void)
{
    byte save_value = 0;
    byte ret = 0;
    hal_uart_print_right_now("[i2c test] --> start\r\n");
    hal_i2c_init();
    save_value = hal_i2c_read(0x1F);
    if (hal_i2c_read(0x1F) != 0x33) {
        ret = 1;
    }
    else {
        save_value = hal_i2c_read(0x06);
        hal_i2c_write(0x06, 0x37);
        if (0x37 == hal_i2c_read(0x06)) {
            hal_i2c_write(0x06, 0x3f);
            if (0x3F == hal_i2c_read(0x06)) {
                hal_i2c_write(0x06, save_value);
                ret = 0;
            }
            else {
                ret = 1;
            }
        }
        else {
            ret = 1;
        }
    }
    if (1 == ret)
        hal_uart_print_right_now("[i2c test] <-- fail\r\n");
    else
        hal_uart_print_right_now("[i2c test] <-- pass\r\n");

    hal_i2c_write(0x06, save_value);

    return ret;
}

dwrd hal_hardware_timer_test_delay_us(dwrd delay_cnt, dwrd timer_id)
{
    dwrd tmp_cnt = 0;

    rd_timer_load_cnt(timer_id) = delay_cnt * TIMER_SYS_DELAY_US;
    rd_timer_control_reg(timer_id) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_UNMASKED | USER_DEFINED | TIMER_ENABLE;
    while (!TIMER_DELAY_FLAG[timer_id]) {
        tmp_cnt++;
        if (tmp_cnt > 10000) {
            rd_timer_load_cnt(timer_id) = DEFAULT_TIMER_VALUE;
            rd_timer_control_reg(timer_id) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_MASKED | USER_DEFINED | TIMER_DISABLE;
            return 1;
        }
    }
    TIMER_DELAY_FLAG[timer_id] = 0;

    return 0;
}

void hal_hardware_timer_interrupt_test_handle(void)
{
    dwrd interrupt_status = rd_timers_int_sts;

    while (rd_timers_eol)
        ;

    for (byte i = 0; i < HARDWARE_TIMER_NUM; i++) {
        if (interrupt_status & (1 << i)) {
            TIMER_DELAY_FLAG[i] = 1;
            rd_timer_load_cnt(i) = DEFAULT_TIMER_VALUE;
            rd_timer_control_reg(i) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_MASKED | USER_DEFINED | TIMER_DISABLE;
        }
    }
}

void hal_hardware_timer_test_init(void)
{
    for (byte i = 0; i < HARDWARE_TIMER_NUM; i++) {
        rd_timer_load_cnt(i) = DEFAULT_TIMER_VALUE;
        rd_timer_control_reg(i) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_MASKED | USER_DEFINED | TIMER_DISABLE;
    }
    hal_intc_irq_isr(IRQ_Timer_Int, IRQ_SYS_PRIORITY, hal_hardware_timer_interrupt_test_handle);
}

dwrd hal_hareware_timer_test(void)
{
    dwrd ret = 0;

    hal_uart_print_right_now("[timer test] --> start\r\n");

    hal_hardware_timer_test_init();
    ret = hal_hardware_timer_test_delay_us(1, TIMER0_SYS);
    ;

    if (1 == ret)
        hal_uart_print_right_now("[timer test] <-- fail\r\n");
    else
        hal_uart_print_right_now("[timer test] <-- pass\r\n");

    return ret;
}

