#ifndef _NAND_CMD_H
#define _NAND_CMD_H

#include <stdint.h>
#include "../share/p4k.h"

//#define CMD_FEATURE_NUM     0x4
#if defined(NAND_BICS5)
#define CMD_READ_ID_NUM     0x6
#elif defined(NAND_YX39070)
#define CMD_READ_ID_NUM     0x6
#endif
#define CMD_READ_PARA_PAGE_NUM 256
#define CMD_READ_UID_NUM 32

/**
 * @brief  nand_init
 * @note   init control by global.h config
 * @retval None
 */
void nand_init(void);

/**
 * @brief  read factory defective
 * @note   this func just support kioxia now
 * @param  blk: select blk
 * @param  bank: select bank
 * @param  chan: select chan
 * @param  planeX: select plane ---- check by single plane
 * @retval 0:good blk  other:bad blk
 */
uint32_t nand_factory_defective_check(uint16_t blk, uint8_t bank, uint8_t chan, uint8_t planeX);

/**
 * @brief  nand training flow
 * @note   for kioxia spec; and this flow now not pass
 * @param  chan: select chan
 * @param  bank: select bank
 * @retval
 */
uint32_t nand_data_training(uint8_t chan, uint8_t bank);

/**
 * @brief  wait erase cmd finsh
 * @note   wait and check register status
 * @param  chan: select channel num
 * @param  bank: select ce num
 * @retval None
 */
uint32_t nand_erase_cmd_wait_and_check(uint8_t chan, uint8_t bank);

/**
 * @brief  wait write cmd finsh
 * @note   wait and check register status
 * @param  chan: select channel num
 * @param  bank: select ce num
 * @retval None
 */
uint32_t nand_write_cmd_wait_and_check(uint8_t chan, uint8_t bank);

/**
 * @brief  wait read cmd finsh
 * @note   wait and check register status
 * @param  chan: select channel num
 * @param  bank: select ce num
 * @retval None
 */
uint32_t nand_read_cmd_wait_and_check(uint8_t chan, uint8_t bank);

/**
 * @brief  nand_cmd: reset
 * @note   cmd_code: 0xFF
 * @param  chan: select channel
 * @param  bank:  select lunce
 * @retval None
 */
void nand_cmd_reset(uint8_t chan, uint8_t bank);

/**
 * @brief  nand_cmd: hard reset
 * @note   cmd_code: 0xFD
 * @param  chan: select channel
 * @param  ce:  select lunce
 * @retval None
 */
void nand_cmd_hard_reset(uint8_t chan, uint8_t bank);

/**
 * @brief  nand_cmd: synchronous reset
 * @note   cmd_code: 0xFC
 * @param  chan: select channel
 * @param  ce:  select lunce
 * @retval None
 */
void nand_cmd_synchronous_reset(uint8_t chan, uint8_t bank);

/**
 * @brief  nand_cmd: reset lun
 * @note   cmd_code: 0xFA
 * @param  chan: select channel
 * @param  ce:  select lunce
 * @param  addr_list:  select address list
 * @param  list_size:  address list size
 * @retval None
 */
void nand_cmd_reset_lun(uint8_t chan, uint8_t bank, uint8_t addr_list[], uint8_t list_size);

/**
 * @brief  nand_cmd: ZQ calibration long(short calibration cannot reach the best code setting. In such case, long calibration should be issued.)
 * @note   cmd_code: 0xF9
 * @param  chan:  select channel
 * @param  ce: select lunce
 * @retval None
 */
void nand_cmd_zq_calibration_long(uint8_t chan, uint8_t bank);

/**
 * @brief  nand_cmd: ZQ calibration short(This short calibration shall be issued after long calibration is done.)
 * @note   cmd_code: 0xD9
 * @param  chan:  select channel
 * @param  ce: select lunce
 * @retval None
 */
void nand_cmd_zq_calibration_short(uint8_t chan, uint8_t bank);

/**
 * @brief  nand_cmd: set feature
 * @note   cmd_code: 0xEF
 * @param  chan: select channel
 * @param  ce:  select lunce
 * @param  nand_adr: Send Address to NAND interface
 * @param  wdt[CMD_FEATURE_NUM]: Write data to NAND interface
 * @retval None
 */
void nand_cmd_set_feature(uint8_t chan, uint8_t bank, uint8_t nand_adr, uint16_t wdt[CMD_FEATURE_NUM]);

/**
 * @brief  nand_cmd: get feature
 * @note   cmd_code: 0xEE
 * @param  chan: select channel
 * @param  ce: select lunce
 * @param  nand_adr: Send Address to NAND interface
 * @param  rdt[CMD_FEATURE_NUM]: Read data from NAND interface
 * @retval None
 */
void nand_cmd_get_feature(uint8_t chan, uint8_t bank, uint8_t nand_adr, uint16_t rdt[CMD_FEATURE_NUM]);

/**
 * @brief  nand_cmd: read unique id
 * @note   cmd_code: 0xED
 * @param  chan: select channel
 * @param  ce: select lunce
 * @param  nand_adr: Send Address to NAND interface
 * @retval Read unique id from NAND interface
 */
uint32_t nand_cmd_read_unique_id(uint8_t chan, uint8_t bank, uint8_t nand_adr, uint8_t rdr[CMD_READ_UID_NUM]);

/**
 * @brief  nand_cmd: ODT configure
 * @note   cmd_code: 0xE2
 * @param  chan: select channel
 * @param  ce: select lunce
 * @param  addr_list[]: select address list
 * @param  addr_num: address list size
 * @param  wdt_list[]: write data list
 * @param  wdt_num: write data num
 * @retval None
 */
void nand_cmd_odt_configure(uint8_t chan, uint8_t bank, uint8_t addr_list[], uint8_t addr_num, uint16_t wdt_list[], uint8_t wdt_num);

/**
 * @brief  nand_cmd: read id
 * @note   cmd_code: 0x90
 * @param  chan: select channel
 * @param  bank: select lunce
 * @param  nand_adr: Send Address to NAND interface
 * @param  rdr[CMD_READ_ID_NUM]: Read data from NAND interface
 * @retval None
 */
void nand_cmd_read_id(uint8_t chan, uint8_t bank, uint8_t nand_adr, uint8_t rdr[CMD_READ_ID_NUM]);


void nand_cmd_read_parameter_page(uint8_t chan, uint8_t bank, uint8_t nand_adr, uint8_t rdr[CMD_READ_PARA_PAGE_NUM]);

/**
 * @brief  nand_cmd: read status enhanced
 * @note   cmd_code: 0x78
 * @param  chan: select channel
 * @param  bank: select lunce
 * @param  addr_list:  select address list
 * @param  list_size:  address list size
 * @retval status val
 */
uint8_t nand_cmd_read_status_enhanced(uint8_t chan, uint8_t bank, uint8_t addr_list[], uint8_t list_size);

/**
 * @brief  nand_cmd: read status lun previous
 * @note   cmd_code: 0x73
 * @param  chan: select channel
 * @param  bank: select lunce
 * @retval status val
 */
uint8_t nand_cmd_read_status_lun_previous(uint8_t chan, uint8_t bank);

/**
 * @brief  nand_cmd: read status lun
 * @note   cmd_code: 0x71
 * @param  chan: select channel
 * @param  bank: select lunce
 * @retval status val
 */
uint8_t nand_cmd_read_status_lun(uint8_t chan, uint8_t bank);

/**
 * @brief  nand_cmd: read status
 * @note   cmd_code: 0x70
 * @param  chan: select channel
 * @param  bank: select lunce
 * @retval status val
 */
uint8_t nand_cmd_read_status(uint8_t chan, uint8_t bank);

/**
 * @brief  nand_cmd: set feature by lun
 * @note   cmd_code: 0xD5
 * @param  chan: select channel
 * @param  bank:  select lunce
 * @param  lun_adr: Send Address to NAND interface(00h:LUN0 01h:LUN1)
 * @param  nand_adr: Send Address to NAND interface
 * @param  wdt[CMD_FEATURE_NUM]: Write data to NAND interface
 * @retval None
 */
void nand_cmd_set_feature_by_lun(uint8_t chan, uint8_t bank, uint8_t lun_adr, uint8_t nand_adr, uint16_t wdt[CMD_FEATURE_NUM]);

/**
 * @brief  nand_cmd: get feature by lun
 * @note   cmd_code: 0xD4
 * @param  chan: select channel
 * @param  bank:  select lunce
 * @param  lun_adr: Send Address to NAND interface(00h:LUN0 01h:LUN1)
 * @param  nand_adr: Send Address to NAND interface
 * @param  wdt[CMD_FEATURE_NUM]: Read data from NAND interface
 * @retval None
 */
void nand_cmd_get_feature_by_lun(uint8_t chan, uint8_t bank, uint8_t lun_adr, uint8_t nand_adr, uint16_t rdt[CMD_FEATURE_NUM]);

uint32_t nand_factory_defective_check(uint16_t blk, uint8_t bank, uint8_t chan, uint8_t planeX);

#endif
