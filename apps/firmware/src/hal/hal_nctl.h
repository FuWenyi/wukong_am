#ifndef _HAL_NCTL_H
#define _HAL_NCTL_H

#include <stdint.h>


typedef struct
{
    // for nand program path: 0: keep data buffer, 1: release data buffer (if program data from DRAM other address or FTL table, must set 0)
    // for nand read path: LDPC index type: 0:min-sum decoder, 1: bit flipping decoder
    uint8_t bmsg_ltp;
    uint8_t bmsg_lid;
    uint8_t bmsg_que;       // ECC decoder output to BMU queue number
    uint8_t en_force_ecc;   // force ECC port info: enable/disable
    uint8_t ecc_port_num;   // port number:0/1

    uint16_t bmsg_dmp;   // dram ptr for dram other address or FTL table
    uint16_t rsvd;

    uint16_t bmsg_bfp;   // BMU buffer pointer in buffer message
    uint32_t cmd_padr;   // cmd address: p4k {blk | page | bank | ce | ch | frag}

    uint8_t cmd_ptr;     // micro code cmd ptr value
    uint8_t cmd_pri;     // 0x0:low_pri 0x1:high_pri
    uint8_t cmd_end;     // 0x0:not_end 0x1:is_end; command end / program unit end, HW can auto set this signal, see 0x44 register
    uint8_t cmd_dmap;    // LBA to PBA direct mapping address
} NCTL_CMD_BASE;

/**
 * @brief  support to send all micro cmd
 * @note   default info is 0, other type should set in base info
 * @param  cmd_base: cmd send param, include all cmd base info
 * @retval None
 */
void hal_nctl_cmd_send_base(NCTL_CMD_BASE cmd_base);

/**
 * @brief  read status control
 * @note   use diffent cmd_ptr to read status
 * @param  cmd_ptr: chose micro cmd
 * READ_STS_70H_CPTR
 * READ_STS_71H_CPTR
 * READ_STS_73H_CPTR
 * READ_STS_78H_CPTR
 * @param  bank: bank num
 * @param  chan: channel number
 * @retval None
 */
void hal_nctl_read_sts(uint8_t cmd_ptr, uint8_t bank, uint8_t chan);

/**
 * @brief  send read status cmd, wait cmd ready and check
 * @note   used diffent read status cmd, to check last send cmd status
 * @param  sts_cmd_ptr: read status cmd ptr
 * @param  bank: bank num
 * @param  chan:  channel number
 * @param  last_cmd_ptr: last send cmd
 * @retval 0: OK    other:fail code
 */
uint32_t hal_nctl_read_status_and_check(uint8_t sts_cmd_ptr, uint8_t  bank, uint8_t chan, uint8_t last_cmd_ptr);

/**
 * @brief  erase block control
 * @note   use diffent cmd_ptr to erase block
 * @param  cmd_ptr: chose micro cmd
 * ERS_CPTR
 * ERS_SINGE_CPTR
 * @param  bank: bank num
 * @param  chan: channel number
 * @param  blk: block num
 * @param  planeX: chose plane(only single use this)
 * @retval None
 */
void hal_nctl_erase_multi_blk(uint8_t cmd_ptr, uint8_t bank, uint8_t chan, uint16_t blk);

void hal_nctl_erase_single_blk(uint8_t cmd_ptr, uint8_t bank, uint8_t chan, uint16_t blk, uint8_t planeX);

/**
 * @brief  program control
 * @note   use diffent cmd_ptr to program
 * @param  cmd_ptr: chose micro cmd
 * PROG_CPTR
 * PROG_SLC_CPTR
 * PROG_SINGE_CPTR
 * PROG_SINGE_SLC_CPTR
 * PROG_CACHE_CPTR
 * PROG_CACHE_SLC_CPTR
 * PROG_CACHE_SINGE_CPTR
 * PROG_CACHE_SINGE_SLC_CPTR
 * @param  buffer_ptr:  BMU buffer ptr for write, 12 bit
 * @param  p4k: {blk | page | bank | ce | ch | frag}
 * @retval None
 */
void hal_nctl_program_buff_4k(uint8_t cmd_ptr, uint16_t buffer_ptr, uint32_t p4k, uint8_t end);

void hal_nctl_program_dram_4k(uint8_t cmd_ptr, uint64_t dram_addr, uint8_t dma_ptr, uint32_t p4k, uint8_t end);

/**
 * @brief  read data control
 * @note   use diffent cmd_ptr to read data
 * @param  cmd_ptr: chose micro cmd
 * READ_CPTR
 * READ_SLC_CPTR
 * READ_PARA_PAGE_CPTR
 * @param  *buffer_ptr: BMU buffer ptr for read, 12 bit;
 * @param  p4k: {blk | page | bank | ce | ch | frag}
 * @retval
 */
void hal_nctl_read_4k_to_buff(uint8_t cmd_ptr, uint16_t buffer_ptr, uint32_t p4k);

/**
 * @brief read 4k from nand to dram
 * @note
 * @param  cmd_ptr: chose micro cmd
 * @param  dram_addr: base dram address
 * @param  p4k: physical 4KB
 * @retval None
 */
void hal_nctl_read_4k_to_dram(uint8_t cmd_ptr, uint32_t dram_addr, uint32_t p4k);

/**
 * @brief dma 4k dram to nand
 * @note
 * @param  cmd_ptr: chose micro cmd
 * @param  dram_addr: base dram address
 * @param  dma_ptr: dma bit sel
 * @param  p4k: physical 4KB
 * @retval None
 */


#endif
