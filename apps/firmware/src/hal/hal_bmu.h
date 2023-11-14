#ifndef _HAL_BMU_H
#define _HAL_BMU_H

#include <stdint.h>
#include "../inc/global.h"
#include "hal_bmu_register.h"

typedef enum
{
    DDMA_DRAM_TO_SRAM = 0,
    DDMA_SRAM_TO_DRAM = 1
} DDMA_BMU_TYPE;

typedef struct
{
    uint16_t org_bfp;  // bmu buffer ptr
    uint16_t bmu_ptr;
    uint16_t rid;
    uint16_t lid;
    uint16_t queue;
    uint32_t padr; // p4k
    uint32_t cmd_tag[4];
} HAL_BMU_READ_ERRQ_CTX;
typedef struct
{
    uint16_t bmu_ptr;
    uint16_t rid;
    uint16_t lid;
    uint16_t crd;
    uint32_t padr; // p4k
    uint32_t cmd_tag[4];
} HAL_BMU_READ_SMRDQ_CTX;
typedef struct
{
    uint16_t bfp;  // bmu buffer ptr
    uint16_t dmp;
    uint32_t meta_data[2];
} HAL_BMU_READ_TRRDQ_CTX;

#define RD_SRAM_PTR_DDMA_PTR(ptr) ((ptr - SRAM_BUFF_POINTER_BASE) << (13 - 4))

#define RAM_4K_BUFF_SIZE 0x1000
#define DWORD_CNT_PER_4K_BUFF (RAM_4K_BUFF_SIZE >> 2)
#define SRAM_META_BUFF_SIZE 160
#define DRAM_META_BUFF_SIZE 0x100
#define LONG_DWORD_CNT_PER_DRAM_META (DRAM_META_BUFF_SIZE >> 3)
#define DRAM_SECTION_SIZE                       0x80000000
/**
 * @brief test chip function for buff alloc
 * @note bad performance,not for io path
 * @param  buff_type: DRAM_BUF: dram buffer, SRAM_BUF: sram buffer, PRTY_BUF: LDPC parity buffer
 * @param  master_type: HOST_GET: host, NAND_GET: nand, DWDM_GET: decw/dram, CPUA_GET: cpua
 * @retval buff_pointer,UINT16_MAX:alloc buff failed
 */
uint16_t hal_bmu_alloc_buff(uint32_t buff_type, uint32_t master_type);

/**
 * @brief  cpu release buff
 * @note read slnk_num/dlnk_num reg to get free buff num
 * @param  buff_ptr: dram/sram buff id
 * @retval None
 */
void hal_bmu_cpu_free_buff(uint16_t buff_ptr);

/**
 * @brief  get sram hw absolute address from buffer ptr
 * @note
 * @param  buff_ptr: 12 bit sram buff ptr
 * @retval sram hw mem address
 */
uint32_t hal_bmu_get_sram_hw_addr(uint16_t buff_ptr);

/**
 * @brief  get sram mem address from buff ptr
 * @note
 * @param  buff_ptr: 12 bit sram buff ptr
 * @retval SRAM mem Address
 */
uint32_t hal_bmu_get_sram_mem_addr(uint16_t buff_ptr);


uint32_t hal_bmu_get_sram_meta_addr(uint16_t buff_ptr);

/**
 * @brief  get sram bfp from cpu sram mem addr
 * @note
 * @param  addr: fw vision mem addr, BMU sram
 * @retval sram buffer ptr
 */
uint16_t hal_bmu_get_sram_bfp_from_cpu_addr(uint32_t addr);

/**
 * @brief  get sram bfp from hw sram mem addr
 * @note
 * @param  addr: hardware absolute mem addr, BMU sram
 * @retval sram buffer ptr
 */
uint16_t hal_bmu_get_sram_bfp_from_hw_addr(uint32_t addr);

/**
 * @brief  get dram bmu data address for cpu from buff ptr
 * @note
 * @param  buff_ptr: 12 bit dram buff ptr
 * @retval DRAM bmu data address for cpu
 */
uint32_t hal_bmu_get_dram_mem_addr(uint16_t buff_ptr);

uint32_t hal_bmu_get_dram_meta_addr(uint16_t buff_ptr);

/**
 * @brief get dram/sram/parity free buff num
 * @note Hardware would allocate several buff during initialization,so the free buff num is less than the total num
 * @param buff_type: DRAM_BUF: dram buffer, SRAM_BUF: sram buffer, PRTY_BUF: LDPC parity buffer
 * @retval free buff num according to buff type
 */
uint16_t hal_bmu_get_free_buff_num_by_type(uint32_t buff_type);

/**
 * @brief get dram/sram/parity usd buff num by master type
 * @note bad performance,not for io path
 * @param  buff_type: DRAM_BUF: dram buffer, SRAM_BUF: sram buffer, PRTY_BUF: LDPC parity buffer
 * @param  master_type: HOST_GET: host, NAND_GET: nand, DWDM_GET: decw/dram, CPUA_GET: cpua
 * @retval used buff num according to buff type and master type
 */
uint16_t hal_bmu_get_used_buff_num_by_type(uint32_t buff_type, uint32_t master_type);

/**
 * @brief check combination of buff_type&master_type is support get used buff num
 * @note bad performance,not for io path
 * @param  buff_type: DRAM_BUF: dram buffer, SRAM_BUF: sram buffer, PRTY_BUF: LDPC parity buffer
 * @param  master_type: HOST_GET: host, NAND_GET: nand, DWDM_GET: decw/dram, CPUA_GET: cpua
 * @retval trun/false
 */
uint32_t hal_bmu_is_support_get_used_buff_num(uint32_t buff_type, uint32_t master_type);

/**
 * @brief get threshold from register of the combination of buff_type&master_type
 * @note bad performance,not for io path
 * @param  buff_type: DRAM_BUF: dram buffer, SRAM_BUF: sram buffer, PRTY_BUF: LDPC parity buffer
 * @param  master_type: HOST_GET: host, NAND_GET: nand, DWDM_GET: decw/dram, CPUA_GET: cpua
 * @retval threshold;UINT16_MAX:the combination of buff_type&master_type has no threshold register
 */
uint16_t hal_bmu_get_buff_threshold_by_type(uint32_t buff_type, uint32_t master_type);

/**
 * @brief set threshold register of the combination of buff_type&master_type,
 * caller make sure the correctness of parameters
 * @note bad performance,not for io path
 * @param  buff_type: DRAM_BUF: dram buffer, SRAM_BUF: sram buffer, PRTY_BUF: LDPC parity buffer
 * @param  master_type: HOST_GET: host, NAND_GET: nand, DWDM_GET: decw/dram, CPUA_GET: cpua
 * @retval none
 */
void hal_bmu_set_buff_threshold(uint32_t buff_type, uint32_t master_type, uint16_t threshold);

uint16_t hal_bmu_get_dram_free_buff_num(void);
uint16_t hal_bmu_get_sram_free_buff_num(void);
uint16_t hal_bmu_get_parity_free_buff_num(void);
uint16_t hal_bmu_get_dram_used_buff_num(uint32_t master_type);
uint16_t hal_bmu_get_sram_used_buff_num(uint32_t master_type);
uint16_t hal_bmu_get_parity_used_buff_num(uint32_t master_type);
uint16_t hal_bmu_get_dram_buff_threshold(uint32_t master_type);
uint16_t hal_bmu_get_sram_buff_threshold(uint32_t master_type);
uint16_t hal_bmu_get_parity_buff_threshold(uint32_t master_type);
void hal_bmu_set_dram_buff_threshold(uint32_t master_type, uint16_t threshold);
void hal_bmu_set_sram_buff_threshold(uint32_t master_type, uint16_t threshold);
void hal_bmu_set_parity_buff_threshold(uint32_t master_type, uint16_t threshold);

/**
 * @brief  wait ddma finish until done.
 * @note   may loop infinite when ddma wrong
 * @retval None
 */
void hal_bmu_wait_ddma_done(void);

uint32_t hal_bmu_config_hw_ddr_to_cpu_ddr(uint64_t hw_dram_addr);

uint32_t hal_bmu_get_cpu_ddr_to_hw_ddr(uint32_t cpu_dram_addr);

/**
 * @brief  start ddma from bmu sram addr to dram addr
 * @note
 * @param  sram_addr: hw addr, bmu absolute sram addr, should be 16byte aligned.
 * @param  dram_addr: hw addr, bmu absolute dram addr, should be 16byte aligned.
 * @param  dma_len: dma length. maximum 8KB, should be 8byte aligned
 * @retval None
 */
uint32_t hal_bmu_start_ddma_sram_to_dram(uint32_t sram_addr, uint64_t dram_addr, uint16_t dma_len);

uint32_t hal_bmu_start_ddma_dram_to_sram(uint64_t dram_hw_addr, uint32_t sram_hw_addr, uint16_t dma_len);

uint32_t hal_bmu_wait_read_buff_vld(void);
uint32_t hal_bmu_wait_read_dram_vld(void);
void hal_bmu_clear_errq(void);
void hal_bmu_clear_smrdq(void);
void hal_bmu_clear_trrdq(void);
uint8_t hal_bmu_get_errq_read_ctx(HAL_BMU_READ_ERRQ_CTX *err_rd_ptr);
uint8_t hal_bmu_get_smrdq_read_ctx(HAL_BMU_READ_SMRDQ_CTX *smrd_ptr);
uint8_t hal_bmu_get_trrdq_read_ctx(HAL_BMU_READ_TRRDQ_CTX *trrd_ptr);

/**
 * @brief wait program done for continous mapu flags
 * @note
 * @param  start_flag: start flag, 0~255
 * @param  length: start flag + length.
 * @retval None
 */
void hal_bmu_wait_program_done(uint8_t start_flag, uint8_t length);

/**
 * @brief clear program done bits for continous mapu flags
 * @note
 * @param  start_flag: start flag, 0~255
 * @param  length: start flag + length.
 * @retval None
 */
void hal_bmu_clear_program_flag(uint8_t start_flag, uint8_t length);

/**
 * @brief  clear multiple program flag reg, bit count may not be multiples of 32
 * @note
 * @param  start_reg_idx: [0~7]
 * @param  bit_cnt: [1~256]
 * @retval None
 */
void hal_bmu_wait_multi_program_flag_reg(uint8_t start_reg_idx, uint8_t bit_cnt);

/**
 * @brief  clear multiple program flag reg, bit count may not be multiples of 32
 * @note
 * @param  start_reg_idx: [0~7]
 * @param  bit_cnt: [1~256]
 * @retval None
 */
void hal_bmu_clear_multi_program_flag_reg(uint8_t start_reg_idx, uint8_t bit_cnt);


#endif
