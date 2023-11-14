#include "hal_timer.h"

/* These variables are only used in the current file, so they are moved here from the external Header file. */
static qwrd system_tick_count = 0;
#define GET_SYS_TICKS (system_tick_count/1000/1000)
static dwrd last_cnt = DEFAULT_TIMER_VALUE;
static dwrd current_cnt = 0;

extern byte hal_intc_irq_isr(byte bChnlNum, byte nPrio, IpIsrFunc isr);

/**
 * @brief
 * @note
 * @retval None
 */
void hal_hardware_timer_init(void)
{
    for (byte i = 0; i < HARDWARE_TIMER_NUM; i++) 
    {
        rd_timer_load_cnt(i) = DEFAULT_TIMER_VALUE;
        rd_timer_control_reg(i) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_MASKED | USER_DEFINED | TIMER_DISABLE;
    }
    hal_intc_irq_isr(IRQ_TIMER, IRQ_SYS_PRIORITY, hal_hardware_timer_interrupt_handle);
}

void hal_hardware_timer_delay_ms(dwrd delay_cnt, dwrd timer_id)
{
    dwrd tmp_cnt = 0;

    rd_timer_load_cnt(timer_id) = delay_cnt * TIMER_SYS_DELAY_MS;
    rd_timer_control_reg(timer_id) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_UNMASKED | USER_DEFINED | TIMER_ENABLE;
    while (!TIMER_DELAY_FLAG[timer_id]) 
    {
        tmp_cnt++;
        if (tmp_cnt > 10000) 
        {
            rd_timer_load_cnt(timer_id) = DEFAULT_TIMER_VALUE;
            rd_timer_control_reg(timer_id) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_MASKED | USER_DEFINED | TIMER_DISABLE;
        }
    }
    TIMER_DELAY_FLAG[timer_id] = 0;
}

void hal_hardware_timer_delay_us(dwrd delay_cnt, dwrd timer_id)
{
    dwrd tmp_cnt = 0;

    rd_timer_load_cnt(timer_id) = delay_cnt * TIMER_SYS_DELAY_US;
    rd_timer_control_reg(timer_id) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_UNMASKED | USER_DEFINED | TIMER_ENABLE;
    while (!TIMER_DELAY_FLAG[timer_id]) 
    {
        tmp_cnt++;
        if (tmp_cnt > 10000) 
        {
            rd_timer_load_cnt(timer_id) = DEFAULT_TIMER_VALUE;
            rd_timer_control_reg(timer_id) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_MASKED | USER_DEFINED | TIMER_DISABLE;
        }
    }
    TIMER_DELAY_FLAG[timer_id] = 0;
}

void hal_hardware_timer_interrupt_handle(void)
{
    // read register to juge which timer trigger interrupt
    dwrd interrupt_status = rd_timers_int_sts;

    while (rd_timers_eol);

    for (byte i = 0; i < HARDWARE_TIMER_NUM; i++) 
    {
        if (interrupt_status & (1 << i)) 
        {
            TIMER_DELAY_FLAG[i] = 1;
            rd_timer_load_cnt(i) = DEFAULT_TIMER_VALUE;
            rd_timer_control_reg(i) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_MASKED | USER_DEFINED | TIMER_DISABLE;
        }
    }
}

/**
 * @brief
 * @note
 * @retval None
 */
byte hal_hardware_timer_start(byte whitch_timer, dwrd cycle_us, dwrd once_or_cycle, call_back_fun_pst_t callback_function)
{
    whitch_timer += 1;
    if (whitch_timer < HARDWARE_TIMER_NUM) 
	{
        rd_timer_control_reg(whitch_timer) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_MASKED | once_or_cycle | TIMER_DISABLE;
        rd_timer_load_cnt(whitch_timer) = (cycle_us * CLK_COUNT_PER_US);
        rd_timer_control_reg(whitch_timer) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_UNMASKED | once_or_cycle | TIMER_ENABLE;

        return 0;
    }
    else 
    {
        return 1;
    }
}
/**
 * @brief
 * @note
 * @retval None
 */
byte hal_hardware_timer_get_count(byte whitch_timer, dwrd *timer_count)
{
    whitch_timer += 1;
    if (whitch_timer < HARDWARE_TIMER_NUM) 
    {
        *timer_count = rd_timer_current_val(whitch_timer) & TIMER_BIT_WIDE / CLK_COUNT_PER_US;
        return 0;
    }
    else 
    {
        return 1;
    }
}

/**
 * @brief
 * @note
 * @retval None
 */
byte hardware_timer_stop(byte whitch_timer)
{
    whitch_timer += 1;
    if (whitch_timer < HARDWARE_TIMER_NUM) 
    {
        rd_timer_control_reg(whitch_timer) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_UNMASKED | TIMER_DISABLE;
        return 0;
    } 
    else 
    {
        return 1;
    }
}

/**
 * @brief
 * @note
 * @retval None
 */
qwrd hal_hardware_timer_get_system_tick_count(byte whitch_timer)
{
    hal_hardware_timer_system_ticks_cacl(whitch_timer);
    return system_tick_count;
}


dwrd hal_hardware_timer_get_system_tick_us(byte whitch_timer)
{
    hal_hardware_timer_system_ticks_cacl(whitch_timer);
    return (dwrd)(system_tick_count * 1000);
}

/**
 * @brief
 * @note
 * @retval None
 */
void hal_hardware_timer_system_ticks_interrupt_handle(void)
{
    system_tick_count++;
}

/**
 * @brief
 * @note
 * @retval None
 */
void hal_hardware_timer_system_ticks_cacl(byte whitch_timer)
{
    current_cnt = rd_timer_current_val(whitch_timer);

    if (last_cnt > current_cnt)
        system_tick_count += (qwrd)(last_cnt - current_cnt) / 125/1000;
    else
        system_tick_count += (qwrd)(DEFAULT_TIMER_VALUE - current_cnt + last_cnt) / 125/1000;

    last_cnt = current_cnt;
}

/**
 * @brief
 * @note
 * @retval None
 */
void hal_hardware_timer_system_tick_start(void)
{
    rd_timer_control_reg(TIMER0_SYS) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_MASKED | TIMER_CYCLE | TIMER_DISABLE;
    rd_timer_load_cnt(TIMER0_SYS) = DEFAULT_TIMER_VALUE;
    rd_timer_control_reg(TIMER0_SYS) = TIMER_0N100PWM_DISABLE | TIMER_PWM_DISABLE | TIMER_INTERRUPT_MASKED | TIMER_CYCLE | TIMER_ENABLE;
}

/**
 * @brief
 * @note
 * @retval None
 */
void hal_hardware_timer_set_system_tick_count(qwrd set_count)
{
    system_tick_count = set_count;
}

