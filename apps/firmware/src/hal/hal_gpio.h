#ifndef __HAL_GPIO_H__
#define __HAL_GPIO_H__

typedef unsigned char 	 	byte;
typedef unsigned short	 	word;
typedef unsigned int		dwrd;
typedef unsigned long long 	qwrd;

#define GPIO_REG_BASE    0x15300000

#define rd_gpio_out_data(port)        (*(volatile dwrd *)(GPIO_REG_BASE + 0x000 + (port)*0xc))  /* Port(A,B,C,D) data register  */
#define rd_gpio_dirction(port)        (*(volatile dwrd *)(GPIO_REG_BASE + 0x004 + (port)*0xc))  /* Port(A,B,C,D) Data Direction Register  */
#define rd_gpio_source(port)          (*(volatile dwrd *)(GPIO_REG_BASE + 0x008 + (port)*0xc))  /* Port(A,B,C,D) data source register  */
#define rd_gpio_int_enable            (*(volatile dwrd *)(GPIO_REG_BASE + 0x030))               /* Interrupt enable register(only port A)  */
#define rd_gpio_int_mask              (*(volatile dwrd *)(GPIO_REG_BASE + 0x034))               /* Interrupt mask register(only port A)   */
#define rd_gpio_int_level             (*(volatile dwrd *)(GPIO_REG_BASE + 0x038))               /* Interrupt level(only port A)   */
#define rd_gpio_int_polarity          (*(volatile dwrd *)(GPIO_REG_BASE + 0x03C))               /* Interrupt polarity(only port A)   */
#define rd_gpio_int_status            (*(volatile dwrd *)(GPIO_REG_BASE + 0x040))               /* Interrupt status(only port A)   */
#define rd_gpio_raw_int_status        (*(volatile dwrd *)(GPIO_REG_BASE + 0x044))               /* Raw interrupt status   */
#define rd_gpio_debounce_enable       (*(volatile dwrd *)(GPIO_REG_BASE + 0x048))               /* Debounce enable   */
#define rd_gpio_int_clear             (*(volatile dwrd *)(GPIO_REG_BASE + 0x04C))               /* clear interrupt register(only port A)   */
#define rd_gpio_in_data(port)         (*(volatile dwrd *)(GPIO_REG_BASE + 0x050 + ((port) << 2))) /* External register   */
#define rd_gpio_sync_level            (*(volatile dwrd *)(GPIO_REG_BASE + 0x060))               /* Synchronization level   */
#define rd_gpio_id_code               (*(volatile dwrd *)(GPIO_REG_BASE + 0x064))               /* GPIO ID code   */
#define rd_gpio_int_bothedge          (*(volatile dwrd *)(GPIO_REG_BASE + 0x068))               /* Interrupt Both Edge type(only port A)   */
#define single_edge                0
#define both_edge                  1
#define rd_gpio_component_ver         (*(volatile dwrd *)(GPIO_REG_BASE + 0x06C)) /* GPIO Component Version  ASCII value 32_31_32_2A = 2.12**/
#define rd_gpio_config_reg2           (*(volatile dwrd *)(GPIO_REG_BASE + 0x070)) /* GPIO Configuration Register 2   */
#define ENCODED_ID_PWIDTH(port, n) ((n) << ((port)*5))
#define rd_gpio_config_reg1           (*(volatile dwrd *)(GPIO_REG_BASE + 0x074)) /* GPIO Configuration Register 1   */
#define INTERRUPT_BOTH_EDGE_TYPE   (1 << 21)
#define ENCODED_ID_WIDTH           (0x1F << 16)
#define GPIO_ID                    (1 << 15)
#define ADD_ENCODED_PARAMS         (1 << 14)
#define DEBOUNCE                   (1 << 13)
#define PORTA_INTR                 (1 << 12)
#define NUM_PORTS(n)               ((n) << 2)

#define PORTS 4

#define LEVEL_SENSITIVE 0
#define EDGE_SENSITIVE  1

#define LOW_POLARITY  0
#define HIGH_POLARITY 1
#define IRQ_SYS_PRIORITY 0x0

#define GPIO_NUM 32

#define get_gpio_addr(gpio_index) (1 << (gpio_index - 16))

typedef struct {
    byte port;
    byte pin;
} gpio_t;

typedef void (*IpIsrFunc)(void);

void hal_gpio_init(void);
byte hal_gpio_get_data(byte gpio_index);
void hal_gpio_set_data(byte gpio_index, byte data);
void hal_gpio_int_init(dwrd gpio_index, byte int_level, byte int_polarity);
void hal_gpio_interrupt_handle(void);

#endif

