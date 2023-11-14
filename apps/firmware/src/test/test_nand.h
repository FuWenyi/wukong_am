#ifndef _TEST_NAND_H
#define _TEST_NAND_H

#include <stdint.h>
#include "../share/p4k.h"

/**
 * @brief  test cmd: reset
 * @note   cmd case: [0xFF, 0xFD, 0xFC, 0xFA]
 * @retval 0: pass
 */
uint32_t test_nand_cmd_reset(void);

/**
 * @brief  test cmd zq calibration
 * @note   step: 1.run cmd zq calibration long  2.run cmd zq calibration short
 * @retval 0: pass
 */
uint32_t test_nand_cmd_zq_calibration(void);

/**
 * @brief  test cmd: set/get feature
 * @note   step: 1.set feature  2.get feature  3.check getval isn't equal setval
 * @retval 0: pass  -1: failed
 */
uint32_t test_nand_cmd_get_set_feature(void);

/**
 * @brief  test cmd: read unique id
 * @note   read unique id; 0 fail or not support
 * @retval 0: pass
 */
uint32_t test_nand_cmd_read_unique_id(void);

/**
 * @brief  test cmd: odt configure
 * @note   set odt configure
 * @retval 0: pass
 */
uint32_t test_nand_cmd_odt_configure(void);

/**
 * @brief  test cmd: read id
 * @note   run case: nand adress 00h/40h
 * @retval 0: pass
 */
uint32_t test_nand_cmd_read_id(void);

/**
 * @brief  test ntcl read parameter page
 * @note   The device returns a JEDEC standard formatted parameter page during the data out phase of
 *         the READ PARAMETER PAGE command when address 40h is inputted.
 * @retval 0:OK  -1:Failed
 */
uint32_t test_nand_cmd_read_parameter_page(void);

/**
 * @brief  test cmd: read id
 * @note   cmd case: [0x78, 0x73, 0x71, 0x70]
 * @retval 0: pass
 */
uint32_t test_nand_cmd_read_status(void);

/**
 * @brief  test cmd: set/get feature by lun
 * @note   step: 1.set feature  2.get feature  3.check getval isn't equal setval
 * @retval 0: pass  -1: failed
 */
uint32_t test_nand_cmd_get_set_feature_by_lun(void);

/**
 * @brief  test read max count
 * @note   Repeatedly read a p4k to see how many times it can be read at most
 * @retval
 */
uint32_t test_nand_read_max_read_count(void);

/**
 * @brief  test read retry
 * @note   read retry test tlc and slc
 * @retval
 */
uint32_t test_nand_read_retry_tlc(void);

uint32_t test_nand_read_retry_slc(void);

/**
 * @brief  test single erase
 * @note   single write and then erase
 * @retval
 */
uint32_t test_single_erase(void);

/**
 * @brief  test cache read/write
 * @note   use cache read/write mode test
 * @retval 0:ok  -1:Failed
 */
uint32_t test_multi_cache_read_write(void);

/**
 * @brief  test single-plane cache read/write
 * @note   use cache read/write mode test
 * @retval 0:ok  -1:Failed
 */
uint32_t test_single_cache_read_write(void);

/**
 * @brief  test slc cache read/write
 * @note   use slc cache read/write mode test
 * @retval 0:ok  -1:Failed
 */
uint32_t test_multi_slc_cache_read_write(void);

/**
 * @brief  test slc single-plane cache read/write
 * @note   use slc cache read/write mode test
 * @retval 0:ok  -1:Failed
 */
uint32_t test_single_slc_cache_read_write(void);

/**
 * @brief  test slc mode read/write
 * @note   use slc read/write cmd test write and read data
 * @retval 0:ok  -1:Failed
 */
uint32_t test_nand_single_slc_read_write(void);

/**
 * @brief  test slc mode read/write
 * @note   use slc read/write cmd test write and read data
 * @retval 0:ok  -1:Failed
 */
uint32_t test_nand_multi_slc_read_write(void);

/**
 * @brief  test single-plane program isn't right
 * @note   write TLC three page, then read the page info check
 * @retval 0:ok  -1:Failed
 */
uint32_t test_nand_single_program_unit(void);

/**
 * @brief  test ntcl write and read one program unit data
 * @note   use tlc mode read/write multi-plane
 * @retval 0:ok  -1:Failed
 */
uint32_t test_nand_multi_program_unit(void);

/**
 * @brief  test ntcl write and read one program unit data
 * @note   use tlc mode read/write multi-plane; but buffer ptr use invalid val
 * @retval 0:ok  -1:Failed
 */
uint32_t test_nand_multi_program_unit_with_auto_buffer(void);

/**
 * @brief  write a block and then read data to check
 * @note   use tlc mode read/write multi-plane;
 * @retval 0:ok  -1:Failed
 */
uint32_t test_nand_read_write_full_block(void);

/**
 * @brief  test ntcl write and read all the nand
 * @note   use tlc mode read/write multi-plane; This use case can only be completed after running for at least two days
 * @retval 0:ok  -1:Failed
 */
uint32_t test_nand_read_write_full_disk(void);

/**
 * @brief  test factory_defective_check
 * @note   print bad blk info list by single erase
 * @retval
 */
uint32_t test_factory_bad_block_print_by_single(void);

/**
 * @brief  use define register to test read/write
 * @note   do not use bmu buffer
 * @retval
 */
uint32_t test_nand_basic_define_rw_test(void);

/**
 * @brief  use struct define register to test read/write
 * @note   do not use bmu buffer
 * @retval
 */
uint32_t test_nand_basic_register_struct_rw_test(void);

void test_base_write_wait_for_read(void);
uint32_t test_base_one_unit_write(uint8_t cmd_ptr, UNIT_PARAM par);
uint32_t test_base_one_unit_read_check(uint8_t cmd_ptr, UNIT_PARAM par);
uint32_t test_base_func_blk_write(uint8_t ce, uint8_t chan, uint16_t blk);
uint32_t test_base_func_blk_read(uint8_t ce, uint8_t chan, uint16_t blk);
uint32_t test_base_factory_blk_write_read(uint8_t ce, uint8_t chan, uint16_t blk);

uint32_t test_nand_erase_interleave(void);
uint32_t test_nand_read_write_interleave(void);
uint32_t test_nand_multi_write_read_error_handle(void);

/**
 * @brief  test ntcl training flow
 * @note   training set and training cmd all flow test
 * @retval 0:OK  -1:Failed
 */
uint32_t test_ntcl_training_flow(void);

/**
 * @brief  nand test interface
 * @note   test case add in this function
 * @retval None
 */
void test_nand_main(void);

void test_nand_init(void);

#endif
