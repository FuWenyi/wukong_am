#ifndef _HAL_UART_H_
#define _HAL_UART_H_

#include <stdint.h>

typedef unsigned char 	 	byte;
typedef unsigned short	 	word;
typedef unsigned int		dwrd;
typedef unsigned long long 	qwrd;

#define UART_REG_BASE    0x15200000

#define rb_rbr_set (*(volatile byte*)(UART_REG_BASE + 0x000)) /* Receive holding register   */
#define rb_iir_set (*(volatile byte*)(UART_REG_BASE + 0x008)) /* Interrupt identification register  */
#define rb_mcr_set (*(volatile byte*)(UART_REG_BASE + 0x010)) /* Modem Control Register  */
#define rb_scr_set (*(volatile byte*)(UART_REG_BASE + 0x01c)) /* Scratch register   */
#define rb_rfl_set (*(volatile byte*)(UART_REG_BASE + 0x084)) /* Receive FIFO Level   */

/* Transmit control Registers   */
#define rb_thr_set (*(volatile byte*)(UART_REG_BASE + 0x000)) /* Transmit holding register  */
#define rb_ier_set (*(volatile byte*)(UART_REG_BASE + 0x004)) /* Interrupt enable register  */
#define rb_fcr_set (*(volatile byte*)(UART_REG_BASE + 0x008)) /* FIFO control register  */
#define rb_lcr_set (*(volatile byte*)(UART_REG_BASE + 0x00c)) /* Line control register  */
#define rb_dll_set (*(volatile byte*)(UART_REG_BASE + 0x000)) /* LSB of baud rate   */
#define rb_dlh_set (*(volatile byte*)(UART_REG_BASE + 0x004)) /* MSB of baud rate   */
#define rb_tfl_set (*(volatile byte*)(UART_REG_BASE + 0x080)) /* Transmit FIFO Level   */

/* Status Registers*/
#define rb_lsr_set (*(volatile byte*)(UART_REG_BASE + 0x014)) /* Line status register   */
#define rb_usr_set (*(volatile byte*)(UART_REG_BASE + 0x07c)) /* UART Status register   */
#define rb_cpr_set (*(volatile byte*)(UART_REG_BASE + 0x0f4)) /* Component parameter register   */
#define rb_iir_set (*(volatile byte*)(UART_REG_BASE + 0x008)) /*  Interrupt Identification Register   */

/* Interrupt enable Register    */
#define IER_RDA 0x01 /* Enable received data available interrupt*/
#define IER_THE 0x02 /* Enable transmitter holding register empty interrupt*/
#define IER_RLS 0x04 /* Enable received data available interrupt*/

/* Interrupt identification Register    */
#define IIR_MS   0x00
#define IIR_nIP  0x01
#define IIR_IBT0 0x02
#define IIR_IBT1 0x04
#define IIR_IBT2 0x08

/* FIFO Control Register    */
#define FCR_CRF 0x02 /* Clear receive FIFO  */
#define FCR_CTF 0x04 /* Clear transmit FIFO */
// TODO
#define FCR_RTF2 0xC0 /* Set Rx trigger at 2 less than full    */
#define FCR_RTH  0x80 /* Set Rx trigger at 1/2 full */
#define FCR_RTQ  0x40 /* Set Rx trigger at 1/4 full */
#define FCR_RT1  0x00 /* Set Rx trigger at 1 */

/* Line Control Register    */
#define LCR_WL5   0x00 /* Word length  */
#define LCR_WL6   0x01
#define LCR_WL7   0x02
#define LCR_WL8   0x03
#define LCR_SB1   0x00 /* Number of stop bits  */
#define LCR_SB1_5 0x04 /* 1.5 -> only valid with 5 bit words   */
#define LCR_SB2   0x04
#define LCR_PN    0x00 /* Parity mode - none   */
#define LCR_PO    0x08 /* Parity mode - odd    */
#define LCR_PE    0x18 /* Parity mode - even   */
#define LCR_PM    0x28 /* Forced "mark" parity */
#define LCR_PS    0x38 /* Forced "space" parity    */
#define LCR_DL    0x80 /* Enable baud rate latch   */

/* Line Status Register */
#define LSR_DR   0x01 /* Data Ready indicator */
#define LSR_OE   0x02 /* Overrun Error indicator */
#define LSR_PE   0x04 /* Parity Error indicator */
#define LSR_FE   0x08 /* Framing error indicator */
#define LSR_BI   0x10 /* Break condition indicator */
#define LSR_THE  0x20 /* Transmitter FIFO empty */
#define LSR_TEMT 0x40 /* Transmitter empty indicator */
#define LSR_FIE  0x80 /* Error condition */

/*  UART Status register */
#define USR_BUSY 0x01 /* UART Busy */
#define USR_TFNF 0x02 /* Transmit FIFO Not Full */
#define USR_TFE  0x04 /* Transmit FIFO Empty */
#define USR_RFNE 0x08 /* Receive FIFO Not Empty */
#define USR_RFF  0x10 /* Receive FIFO Full */

/*  UART CPR register */
#define CPR_UART_ADD_ENCODED_PARAMS (1<<12)
#define CPR_FIFO_STAT_ENABLE        (1 << 10)
#define CPR_ADDITIONAL_FEAT_ENABLE  (1 << 8)
#define CPR_FIFO_MODE_256           (0x10 << 16)
#define CPR_FIFO_MODE_MASK               (0xff00ffff)



#define CONFIG_UART_TX_BUF_SIZE 256

#define CONFIG_UART_BARDRATE 38400

void hal_uart_init(void);
void hal_fv_uart_print(const char *fmt, ...);
void hal_uart_print_right_now(const char *fmt, ...);
void hal_uart_print_from_fifo(void);
void hal_uart_putc(byte c);
void hal_uart_puts(const char *buf);
void hal_uart_read_data(void);

void hal_put_hex(byte b_hex);
void hal_put_2hex(word w_hex);
void hal_put_4hex(dwrd dw_hex);
void hal_mem_dump(dwrd addr, dwrd count);
void hal_mem_dump_right_now(dwrd addr, dwrd count, byte right_now);

word hal_get_uart_buffer(char *buf, dwrd buflen);

#define UART_RX

#ifdef UART_RX
void hal_uart_rx_cmd_list_init(void);
void hal_uart_rx_cmd(void);
void hal_testrx(dwrd argc, dwrd arg[]);

typedef void (*UART_CALL_FUNC)(dwrd, dwrd[]);

typedef struct {
    char *cmd;
    UART_CALL_FUNC func;
} RX_CMD;

static const RX_CMD uart_rx_cmd_table[] =
    {
        {"testrx", hal_testrx}};

#define UART_CMD_BUFFER_SIZE 256
#define UART_RX_CMD_SIZE     64
#define UART_RX_ARG_SIZE     8

static word cmd_buffer_tail_index = 0;
static word cmd_buffer_current_index = 0;
static word rx_cmd_index = 0;
static char uart_cmd_buffer[UART_CMD_BUFFER_SIZE] = {0};
static const RX_CMD *uart_cmd_list[UART_RX_CMD_SIZE] = {0};
static char rx_cmd[UART_RX_CMD_SIZE] = {0};
#endif



#endif  //_UART_H_

