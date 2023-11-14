#include "test_public.h"
#include "test_nand.h"
#include "../inc/global.h"
#include "test_misc.h"

#define TIMER_TEST
#define I2C_TEST
#define WDT_TEST
#define SPI_TEST
#define GPIO_TEST
#define NAND_TEST

extern void fv_uart_init(uint32_t baudrate);
extern void fv_nand_init(void);

void test_ut_main(void)
{
    fv_uart_init(115200);
    fv_uart_print("ssd power on\r\n");

#ifdef TIMER_TEST
    hal_hareware_timer_test();
#endif

#ifdef I2C_TEST
    hal_i2c_test();
#endif

#ifdef SPI_TEST
    hal_spi_test();
#endif

#ifdef GPIO_TEST
    hal_gpio_test();
#endif

#ifdef NAND_TEST
    test_nand_main();
#endif

    fv_uart_print("test end loop ......\r\n");
    //UT stop loop
    while(1)
    {
    }
}

