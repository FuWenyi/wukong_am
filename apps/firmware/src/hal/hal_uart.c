#include <hal_uart.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef CONFIG_UART_TX_ENABLE_QUENE
#define CONFIG_UART_TX_QUENE_SIZE (4 * 1024)
#else
#define CONFIG_UART_TX_QUENE_SIZE (1)
#endif
#define CONFIG_UART_RX_BUF_SIZE 256
#define CONFIG_UART_FIFO_LENGTH 16
#define NVME_RECV_BUFFER_LENGTH (1 * 1024)

typedef struct
{
  word size;
  word recv_count;
  word head;
  word tail;
  char recv_data[1];
}uart_quene;

typedef struct
{
    word size;
    word recv_count;
    word head;
    word tail;
    word min_size;
    byte data[CONFIG_UART_RX_BUF_SIZE];
} rx_buffer;

typedef struct 
{
    word size;
    word recv_count;
    word head;
    word tail;
    word min_size;
    byte data[CONFIG_UART_TX_QUENE_SIZE];
} tx_buffer;

typedef struct
{
    word size;
    word recv_count;
    word head;
    word tail;
    word min_size;
    byte data[1];
} quene_buffer;

typedef struct 
{
    word size;
    word recv_count;
    word head;
    word tail;
    word min_size;
    char data[NVME_RECV_BUFFER_LENGTH];
} nvme_buffer;

static nvme_buffer g_nvme_buffer;

static byte hex_to_ascii[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
static rx_buffer rb;
static tx_buffer tb;

void hal_quene_init(uart_quene* quene,word size)
{
  quene->size = size;
  quene->recv_count = 0;
  quene->head = 0;
  quene->tail = 0;
}

byte hal_quene_is_empty(quene_buffer *quene)
{
    return quene->recv_count == 0;
}

byte hal_quene_is_full(quene_buffer *quene, word len)
{
    return quene->recv_count * quene->min_size + len > quene->size;
}

void hal_quene_clear(quene_buffer *quene)
{
    quene->recv_count = 0;
    quene->head = 0;
    quene->tail = 0;
}

word hal_quene_get_count(quene_buffer *quene)
{
    return quene->recv_count;
}

byte hal_quene_push_ex(quene_buffer *quene, char *data, word len, byte is_overwrite, byte is_block)
{
    int count = 0;
    if (len % quene->min_size != 0)
        return false;
    if (hal_quene_is_full(quene, len)) 
    {
        if (is_overwrite) 
        {
            quene->head = (quene->head + len) % quene->size;
            quene->recv_count -= len / quene->min_size;
        }
        else 
        {
            if (is_block) 
            {
                // 阻塞
                while (hal_quene_is_full(quene, len)) 
                {
                    if (count++ >= 10)
                        return false;
                }
            }
            else 
            {
                // 不阻塞
                return false;
            }
        }
    }

    if (quene->tail + len > quene->size) 
    {
        memcpy(quene->data + quene->tail, data, quene->size - quene->tail);
        memcpy(quene->data, data + quene->size - quene->tail, len - (quene->size - quene->tail));
    }
    else 
    {
        memcpy(quene->data + quene->tail, data, len);
    }
    quene->tail = (quene->tail + len) % quene->size;
    quene->recv_count += len / quene->min_size;
    return true;
}

byte hal_quene_push(quene_buffer *quene, char *data, word len)
{
    return hal_quene_push_ex(quene, data, len, 0, 0);
}

word hal_quene_pop(quene_buffer *quene, char *data, word len)
{
    word data_len = 0;
    if (hal_quene_is_empty(quene)) 
    {
        return data_len;
    }
    if (len % quene->min_size != 0)
        return data_len;

    if (len > quene->recv_count * quene->min_size)
        len = quene->recv_count * quene->min_size;
    if (quene->head + len > quene->size) 
    {
        memcpy(data, quene->data + quene->head, quene->size - quene->head);
        memcpy(data + quene->size - quene->head, quene->data, len - (quene->size - quene->head));
    }
    else 
    {
        memcpy(data, quene->data + quene->head, len);
    }
    data_len = len;
    quene->head = (quene->head + len) % quene->size;
    quene->recv_count -= len / quene->min_size;
    return data_len;
}

void hal_uart_buffer_init()
{
  hal_quene_init((uart_quene*)&rb,CONFIG_UART_RX_BUF_SIZE);
  hal_quene_init((uart_quene*)&tb,CONFIG_UART_TX_QUENE_SIZE);
}

void hal_uart_init()
{
    dwrd sys_clk = 125000000;
    word uart_divider = 0;

    while ((rb_usr_set & USR_BUSY) == USR_BUSY)
        ;

    uart_divider = sys_clk / 16 / CONFIG_UART_BARDRATE;
    rb_lcr_set = LCR_DL;
    rb_dll_set = uart_divider & 0xFF;
    rb_dlh_set = (uart_divider >> 8) & 0xFF;
    rb_lcr_set = LCR_SB1 | LCR_PN | LCR_WL8;

    rb_mcr_set = 0;
    rb_fcr_set = 0x07 | (2 << 6) | (3 << 4);
    //    rb_ier = SET_BIT(0) | SET_BIT(2);

    //log_init();
    hal_uart_buffer_init();
}

// Transmit FIFO reset
void hal_uart_tx_fifo_reset()
{
    // while ((rb_lsr & LSR_TEMT) == 0)
    //     ;
    // rb_fcr |= FCR_CTF;
}

void hal_uart_put(byte c)
{
    rb_thr_set = c;
}

void hal_uart_putc(byte c)
{
#ifdef CONFIG_UART_TX_ENABLE_QUENE
    hal_quene_push((quene_buffer *)&tb, &c, 1);
#else
    while ((rb_lsr_set & LSR_TEMT) == 0)
        ;
    hal_uart_put(c);
    hal_uart_tx_fifo_reset();
#endif
}

void hal_uart_puts(const char *buf)
{
    dwrd i, data_length = strlen(buf);

#ifdef CONFIG_UART_TX_ENABLE_QUENE
    hal_quene_push((quene_buffer *)&tb, buf, data_length);
#else
    dwrd count;
    while (data_length > 0 && *buf != 0) 
    {
        while ((rb_lsr_set & LSR_TEMT) == 0)
            ;
        count = data_length <= CONFIG_UART_FIFO_LENGTH ? data_length : CONFIG_UART_FIFO_LENGTH;
        for (i = 0; i < count; i++) 
        {
            hal_uart_put(*buf++);
        }
        data_length -= count;
    }
    hal_uart_tx_fifo_reset();
#endif
}

void hal_uart_puts_right_now(char *buf)
{
    dwrd i, data_length = strlen(buf);
    dwrd count;
    while (data_length > 0 && *buf != 0) 
    {
        while ((rb_lsr_set & LSR_TEMT) == 0)
            ;
        count = data_length <= CONFIG_UART_FIFO_LENGTH ? data_length : CONFIG_UART_FIFO_LENGTH;
        for (i = 0; i < count; i++) 
        {
            hal_uart_put(*buf++);
        }
        data_length -= count;
    }
    hal_uart_tx_fifo_reset();
}

void hal_put_hex(byte b_hex)
{
    hal_uart_putc(hex_to_ascii[(b_hex >> 4) & 0x0f]);
    hal_uart_putc(hex_to_ascii[b_hex & 0x0f]);
}

void hal_put_2hex(word w_hex)
{
    hal_put_hex((byte)((w_hex >> 8) & 0x00ff));
    hal_put_hex((byte)(w_hex & 0x00ff));
}

void hal_put_4hex(dwrd dw_hex)
{
    hal_put_2hex((word)((dw_hex >> 16) & 0x0000ffff));
    hal_put_2hex((word)(dw_hex & 0x0000ffff));
}

void hal_mem_dump(dwrd addr, dwrd count)
{
    byte bValue;
    dwrd dwCount;
    for (dwCount = 0; dwCount < count; dwCount++) 
    {
        bValue = (*(byte *)(addr + dwCount));
        if (dwCount % 16 == 0) 
        {
            hal_put_4hex(addr + dwCount);
            hal_uart_puts("h: ");
        }
        hal_put_hex(bValue);
        hal_uart_puts(" ");
        if (dwCount != 0 && dwCount % 16 == 15)
            hal_uart_putc('\r');
    }
}

void hal_mem_dump_right_now(dwrd addr, dwrd count, byte right_now)
{
    dwrd i;
    dwrd *p = (dwrd *)addr;
    if (right_now) 
    {
        for (i = 0; i < count; i++) 
        {
            if (i % 10 == 0) 
            {
                hal_uart_print_right_now("\r\n0x%08X : ", p);
            }
            hal_uart_print_right_now("%08x  ", *p++);
        }
        hal_uart_puts_right_now("\r\n");
    }
    else 
    {
        for (i = 0; i < count; i++) 
        {
            if (i % 10 == 0) 
            {
                hal_fv_uart_print("\r\n0x%08X : ", p);
            }
            hal_fv_uart_print("%08x  ", *p++);
        }
        hal_uart_puts("\r\n");
    }
}

word hal_get_uart_buffer(char *buf, dwrd buflen)
{
    return hal_quene_pop((quene_buffer *)&rb, buf, buflen);
}

void hal_uart_read_data(void)
{
    char temp;
    while ((rb_lsr_set & LSR_DR) == LSR_DR) 
    {
        temp = rb_rbr_set;
        hal_quene_push((quene_buffer *)&rb, &temp, 1);
    }
}

byte hal_uart_getc(void)
{
    byte temp = 0;
    if ((rb_lsr_set & LSR_DR) == LSR_DR) 
    {
        temp = rb_rbr_set;
    }
    return temp;
}

/**
 * @brief  uart中断处理函数
 */
void hal_uart_irq_handler(void)
{
    byte temp = temp;
    switch (rb_iir_set & 0x0f) 
    {
        case 0x06:
            while (rb_rfl_set) 
            {
                temp = rb_rbr_set;
            }
            break;
        case 0x04:
            hal_uart_read_data();
            break;
        case 0x0C:
            hal_uart_read_data();
            break;
        default:
            break;
    }
    rb_lsr_set |= rb_lsr_set;
}

void hal_fv_dbg_loop(byte b_flag)
{
    return;
}

void hal_uart_print_from_fifo()
{
#ifdef CONFIG_UART_TX_ENABLE_QUENE
    dwrd i, count, data_length = quene_get_count((quene_buffer *)&tb);
    byte buf[CONFIG_UART_FIFO_LENGTH];
    while (data_length > 0) 
    {
        while ((rb_lsr_set & LSR_TEMT) == 0)
            ;
        count = data_length <= CONFIG_UART_FIFO_LENGTH ? data_length : CONFIG_UART_FIFO_LENGTH;
        hal_quene_pop((quene_buffer *)&tb, buf, count);
        for (i = 0; i < count; i++) 
        {
            hal_uart_put(buf[i]);
        }
        data_length -= count;
    }
    hal_uart_tx_fifo_reset();
#endif
}

void hal_fv_uart_print(const char *fmt, ...)
{
    char buffer[CONFIG_UART_TX_BUF_SIZE];
    va_list argptr;

    memset(buffer, 0, CONFIG_UART_TX_BUF_SIZE);
    va_start(argptr, fmt);
    vsnprintf(buffer, CONFIG_UART_TX_BUF_SIZE, fmt, argptr);
    va_end(argptr);
    hal_uart_puts(buffer);
}

void hal_uart_print_right_now(const char *fmt, ...)
{
    char buffer[CONFIG_UART_TX_BUF_SIZE];
    va_list argptr;

    memset(buffer, 0, CONFIG_UART_TX_BUF_SIZE);
    va_start(argptr, fmt);
    vsnprintf(buffer, CONFIG_UART_TX_BUF_SIZE, fmt, argptr);
    va_end(argptr);
    hal_uart_puts_right_now(buffer);
}

void hal_fv_uart_chk(void)
{
    return;
}

void rest_nvme_buffer(nvme_buffer *nvme, dwrd size)
{
    nvme->recv_count = 0;
    nvme->head = nvme->tail = 0;
}

void write_to_nvme_buffer(char *buffer, dwrd len)
{
    hal_quene_push((quene_buffer *)&g_nvme_buffer, buffer, len);
}

#define MIN_NVME_MUL_SIZE 32
void reverse(char *str)
{
    int i = 0, j = strlen(str) - 1;
    while (i < j) 
    {
        char tmp = str[i];
        str[i] = str[j];
        str[j] = tmp;
        i++;
        j--;
    }
}

void fw_itoa(int num, char *str)
{
    int i = 0;
    // 处理负数
    if (num < 0) 
    {
        num = -num;
        str[i++] = '-';
    }
    // 将数字转换为字符串
    do 
    {
        str[i++] = num % 10 + '0';
        num /= 10;
    } while (num != 0);

    str[i] = '\0';
    // 翻转字符串
    reverse(str);
}

dwrd read_from_nvme_buffer(char *buffer, dwrd len)
{
    dwrd i, count, size;
    char digit[MIN_NVME_MUL_SIZE];
    char buf[MIN_NVME_MUL_SIZE];
    memset(digit, 0, MIN_NVME_MUL_SIZE);

    count = hal_quene_get_count((quene_buffer *)&g_nvme_buffer);

    if (count + MIN_NVME_MUL_SIZE > len) 
    {
        count = len - MIN_NVME_MUL_SIZE;
    }
    fw_itoa(count, digit);

    sprintf(digit, "%s\r\n", digit);
    size = MIN_NVME_MUL_SIZE + count;
    if (size < len) 
    {
        len = size;
    }

    // The length of the data read from the NVMe buffer must be a multiple of min_size.
    if (len % MIN_NVME_MUL_SIZE != 0) 
    {
        len = (len / MIN_NVME_MUL_SIZE + 1) * MIN_NVME_MUL_SIZE;
    }

    memcpy(buffer, digit, MIN_NVME_MUL_SIZE);

    count = len / MIN_NVME_MUL_SIZE;
    for (i = 1; i < count; i++) 
    {
        hal_quene_pop((quene_buffer *)&g_nvme_buffer, buf, MIN_NVME_MUL_SIZE);
        memcpy(buffer + i * MIN_NVME_MUL_SIZE, buf, MIN_NVME_MUL_SIZE);
        memset(buf, 0, MIN_NVME_MUL_SIZE);
    }
    return len;
}

dwrd get_nvme_buffer_length(void)
{
    return g_nvme_buffer.recv_count;
}

#ifdef UART_RX
void hal_testrx(dwrd argc, dwrd arg[])
{
    hal_uart_print_right_now("fortest arg0 0x%x arg1 0x%x\r\n", arg[0], arg[1]);
}

void hal_uart_rx_cmd_list_init(void)
{
    byte table_index = 0;

    for (table_index = 0; table_index < UART_RX_CMD_SIZE; table_index++) 
    {
        uart_cmd_list[table_index] = &uart_rx_cmd_table[table_index];
        if (uart_cmd_list[table_index]->cmd == NULL || uart_cmd_list[table_index]->func == NULL)
            break;
    }
}

word hal_uart_receive_data(void)
{
    while ((rb_lsr_set & LSR_DR) == LSR_DR) 
    {
        uart_cmd_buffer[cmd_buffer_tail_index++] = rb_rbr_set;
    }

    return cmd_buffer_tail_index;
}

void hal_uart_rx_decode_arg(char *argv[], byte argc, dwrd *arg)
{
    byte parse_index = 0;
    char *endptr;

    for (parse_index = 1; parse_index < argc; parse_index++) 
    {
        arg[parse_index - 1] = strtoul(argv[parse_index], &endptr, 16);
    }
}

byte hal_uart_rx_decode_cmd(char *cmd, char *argv[])
{
    byte parse_index = 0;

    while (parse_index < UART_RX_CMD_SIZE) 
    {
        if ((*cmd == '\r') || (*cmd == '\n')) 
        {
            argv[parse_index] = NULL;
            return parse_index;
        }
        else if (*cmd == ' ') 
        {
            ++cmd;
        }
        else 
        {
            argv[parse_index++] = cmd;
            while ((*cmd != ' ') && (*cmd != '\r') && (*cmd != '\n'))
                ++cmd;
        }
    }

    return parse_index;
}

byte hal_compare_cmd(char *current_cmd, char *cmp_cmd)
{
    while (*cmp_cmd != NULL) 
    {
        if (*current_cmd != *cmp_cmd)
            return 0;
        ++current_cmd;
        ++cmp_cmd;
    }

    return 1;
}

byte hal_search_cmd(char *cmd)
{
    byte cmd_index = 0;
    while (cmd_index < UART_RX_CMD_SIZE) 
    {
        if (uart_cmd_list[cmd_index]->cmd == NULL)
            return UART_RX_CMD_SIZE;
        if (hal_compare_cmd(cmd, uart_cmd_list[cmd_index]->cmd))
            break;
        else
            cmd_index++;
    }

    return cmd_index;
}

void hal_uart_rx_execute(char *cmd)
{
    char *argv[UART_RX_ARG_SIZE + 1];
    byte argc, cmd_index;
    dwrd arg[UART_RX_ARG_SIZE];

    argc = hal_uart_rx_decode_cmd(cmd, argv);
    hal_uart_rx_decode_arg(argv, argc, arg);

    if (argc < UART_RX_ARG_SIZE)
        cmd_index = hal_search_cmd(argv[0]);
    else
        hal_uart_print_right_now("error arg cnt\r\n");

    if (cmd_index < UART_RX_CMD_SIZE)
        uart_cmd_list[cmd_index]->func(argc - 1, arg);
    else
        hal_uart_print_right_now("no search func\r\n");

    memset((void *)rx_cmd, 0, UART_RX_CMD_SIZE);
}

void hal_uart_rx_buffer_init(void)
{
    cmd_buffer_current_index = 0;
    cmd_buffer_tail_index = 0;
    memset((void *)uart_cmd_buffer, 0, UART_CMD_BUFFER_SIZE);
}

void hal_uart_rx_decode(void)
{
    char rx_data;

    for (; cmd_buffer_current_index < cmd_buffer_tail_index; cmd_buffer_current_index++) 
    {
        rx_data = uart_cmd_buffer[cmd_buffer_current_index];
        switch (rx_data) 
        {
            case '\n':
            case '\r':
                rx_cmd[rx_cmd_index++] = rx_data;
                hal_uart_rx_execute(rx_cmd);
                rx_cmd_index = 0;
                break;
            case '\b':
                rx_cmd_index--;
                rx_cmd[rx_cmd_index] = NULL;
                break;
            default:
                rx_cmd[rx_cmd_index++] = rx_data;
        }
    }
    hal_uart_rx_buffer_init();
}

void hal_uart_rx_cmd(void)
{
    hal_uart_receive_data();
    if (cmd_buffer_current_index != cmd_buffer_tail_index) 
    {
        hal_uart_rx_decode();
    }
}
#endif
