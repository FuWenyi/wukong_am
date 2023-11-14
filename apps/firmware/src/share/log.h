#ifndef __LOG_H__
#define __LOG_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

typedef enum log_level
{
    debug,
    info,
    event,
    warning,
    error,
    fatal,
    log_level_max
} log_level_t;

#define UART_MODE_REG

typedef enum
{
    LOG_SAVE_TYPE_NORMAL,
    LOG_SAVE_TYPE_IMPORTANT,
    LOG_SAVE_TYPE_MAX
} log_save_type_p;

#define COREDUMP_SAVE_TYPE  (1)
// #define CONFIG_UART_TX_ENABLE_QUEUE

typedef struct log_mgr
{
    uint32_t write_log_len[LOG_SAVE_TYPE_MAX + COREDUMP_SAVE_TYPE];
    uint32_t read_log_len[LOG_SAVE_TYPE_MAX + COREDUMP_SAVE_TYPE];
    uint8_t log_mode;
    uint8_t print_level;
    uint8_t normal_record_level;
    uint8_t important_record_level;
} log_mgr_t;

typedef enum log_set
{
    log_mode_set,
    log_print_level_set,
    log_normal_record_level_set,
    log_important_record_level_set,
    log_set_max,
} log_set_t;

extern int8_t *log_level_string[];
extern log_mgr_t g_log_mgr;

#define ONE_LOG_MSG_PERSIST_SIZE                (128)
#define ONE_LOG_MSG_TAIL_RESERVE_SIZE           (2) //:/r /n /0
#define ONE_LOG_MSG_END_POSITION                (ONE_LOG_MSG_PERSIST_SIZE - ONE_LOG_MSG_TAIL_RESERVE_SIZE)
#define ONE_LOG_PRINT_BUF_SIZE                  (255)


#define ONE_LOG_MSG_HEAD_AND_TAIL_SIZE          (60)

#define ONE_LOG_MSG_HEAD_EXCEPT_LEVEL_SIZE      (ONE_LOG_MSG_HEAD_AND_TAIL_SIZE - ONE_LOG_MSG_TAIL_RESERVE_SIZE)
#define ONE_LOG_MSG_VALID_STR_SIZE              (ONE_LOG_MSG_PERSIST_SIZE - ONE_LOG_MSG_HEAD_AND_TAIL_SIZE)
#define ONE_LOG_MSG_EXCEPT_LEVEL_SIZE           (ONE_LOG_MSG_PERSIST_SIZE)
#define ONE_LOG_MSG_WRITE_CORE_FIFO_SIZE        (ONE_LOG_MSG_PERSIST_SIZE - PACKAGE_HEAD_NUM)
#define OPEN_LOG_UART (1<<0)
#define OPEN_LOG_NVME (1<<1)

#define OPEN_LOG_CNT_PER_BLK                    (PU_QNTY*ONE_DIE_SLC_PROU_DATA_SIZE/ONE_LOG_MSG_PERSIST_SIZE)

#define LOG_PROGRAM_UNIT_SIZE (4096)

void log_print_and_record(int8_t level, int8_t *whitch_core, int8_t *time, int8_t *level_string, const char *fun, char *fmt, ...);

#define log_print(level, fmt, ...)  fv_uart_print((char *)fmt, ##__VA_ARGS__)//log_print_and_record(level, get_core_name(), get_datetime_str(), log_level_string[level], __func__, (char *)fmt, ##__VA_ARGS__)

extern int8_t log_print_buf[];
extern int8_t log_msg_buf[];
typedef void (* print_packaging_to_msg_callback_fun)(int8_t level, int8_t *msg_buf, uint16_t len);

void set_log_mode(uint8_t set_mode, uint8_t set_val);
uint32_t get_nvme_buffer_length(void);
uint32_t read_log_from_nvme_uart_fifo(int8_t *buffer, uint32_t len);
void record_coredump_log(void);
void write_coredump_log(void);
int8_t read_log_from_nand(uint8_t type, uint32_t offset);
//void write_log_to_nand(uint8_t type, root_dram_buf_mgr_t *p_dram_buf_mgr);

void gather_coredump_information(void);
uint8_t get_log_mode(uint8_t set_mode);
void get_user_top_of_stacks(uint32_t *sp);
void print_packaging_to_msg_form(int8_t level, int8_t *msg_head, uint8_t log_head_len, int8_t *print_buf, uint8_t print_str_len, print_packaging_to_msg_callback_fun fun);
uint32_t format_string(int8_t *buffer, uint32_t bufferSize, int8_t *format, ...);
void read_log_first_4k(uint8_t log_type, uint8_t **buffer);
void get_log_buf_point(uint8_t num, uint8_t **buffer);
void log_uart_init(void);
void log_space_init(void);

#endif
