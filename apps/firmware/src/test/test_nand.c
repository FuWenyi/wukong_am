#include <stdlib.h>
#include "test_nand.h"
#include "nand_cmd.h"
#include "../share/log.h"
#include "../share/p4k.h"
#include "../share/error_code.h"
#include "../share/nand_para.h"
#include "../inc/global.h"
#include "../inc/register.h"
#include "../hal/hal_bmu.h"
#include "../hal/hal_nctl.h"
#include "../hal/hal_nctl_register.h"

extern void fv_nand_init(void);
extern void fv_ftl_pre(void);

#define PU_QNTY            (PAGE_QNTY / 3)
#if defined(NAND_YX39070)
#define MAPU_NUM           72
#define META_NUM           32
#else
#define MAPU_NUM           24
#define META_NUM           32
#endif
#define META_CRC_LEN       4
#define TEST_WRITE_4K_SIZE (4096)
#define TEST_READ_4K_SIZE  (TEST_WRITE_4K_SIZE)  // the 4K read(last 4byte is use to auto CRC check)

#define TEST_BLOCK   0x3
#define TEST_CHANNEL 0x0
#define TEST_BANK    0x0
#define TLC_BLOCK_SIZE (16 * 1024 * PAGE_QNTY)

#define CMD_STS_ALL_FIN     0x8
#define CMDQ_VLD_ALL_INVLD  0x0
#define QUE_WPTR_ALL_EMPTY  0x0
#define QUE_RDY_ALL_READY   0xffffffff
#define QABT_FSM_IDLE_ST    0x0
#define REG_RDS_DAT_READY   (1 << 6)
#define REG_RDS_DAT_PE_FAIL (1 << 0)


#define NAND_TEST_FUNC(func)                    \
do {                                            \
    log_print(info, "\r\n=====start "#func"====="); \
    uint32_t ret = func();                       \
    log_print(info, "test ret: %d\r\n", ret);       \
} while (0)

#define UN_CLERA_TEST             // these test case maybe some wrong; Impact other test cases after running

void test_nand_init(void)
{
#ifdef NAND_YX39070
	rb_cmd_tp01 = 0xf8;
	rb_cmd_tp11 = 0xf0;
    rb_cmd_tp12 = 0x01;
#else
    rb_cmd_tp01 = 0x78;
    rb_cmd_tp11 = 0xf0;
#endif
}

void test_nand_main(void)
{
    fv_nand_init();
    fv_ftl_pre();
    test_nand_init();

    NAND_TEST_FUNC(test_single_erase);
    NAND_TEST_FUNC(test_nand_single_program_unit);
    NAND_TEST_FUNC(test_nand_single_slc_read_write);

}

uint32_t test_nand_cmd_reset(void)
{
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    {
        nand_cmd_reset(TEST_CHANNEL, TEST_BANK);
        uint8_t status = nand_cmd_read_status(TEST_CHANNEL, TEST_BANK);
        if (status != 0xE0)
        {
            log_print(info, "nand_cmd_reset fail: %#x\r\n", status);
            return RET_ERR;
        }
    }
    {
        nand_cmd_synchronous_reset(TEST_CHANNEL, TEST_BANK);
        uint8_t status = nand_cmd_read_status(TEST_CHANNEL, TEST_BANK);
        if (status != 0xE0)
        {
            log_print(info, "nand_cmd_synchronous_reset fail: %#x\r\n", status);
            return RET_ERR;
        }
    }
    {
#if defined(NAND_BICS5)
        uint8_t addr_list[] = { 0, 1, 2 };
#elif defined(NAND_YX39070)
        uint8_t addr_list[] = { 0, 1, 2, 3 };
#endif
        nand_cmd_reset_lun(TEST_CHANNEL, TEST_BANK, addr_list, ITEM_OF(addr_list));
        uint8_t status = nand_cmd_read_status(TEST_CHANNEL, TEST_BANK);
        if (status != 0xE0)
        {
            log_print(info, "nand_cmd_reset_lun fail: %#x\r\n", status);
            return RET_ERR;
        }
    }
    {
#if defined(NAND_BICS5)
        uint8_t addr_list[] = { 0, 1, 2 };
#elif defined(NAND_YX39070)
        uint8_t addr_list[] = { 0, 1, 2, 3 };
#endif
        uint8_t status_en = nand_cmd_read_status_enhanced(TEST_CHANNEL, TEST_BANK, addr_list, ITEM_OF(addr_list));
        if (status_en != 0xE0)
        {
            log_print(info, "nand_cmd_hard_reset fail: %#x\r\n", status_en);
            return RET_ERR;
        }
        // before FDh(hard_reset), the cmd mast send 78h(read_status_enhanced)
        nand_cmd_hard_reset(TEST_CHANNEL, TEST_BANK);
        uint8_t status = nand_cmd_read_status(TEST_CHANNEL, TEST_BANK);
        if (status != 0xE0)
        {
            log_print(info, "nand_cmd_hard_reset fail: %#x\r\n", status);
            return RET_ERR;
        }
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_cmd_zq_calibration(void)
{
    {
        nand_cmd_zq_calibration_long(TEST_CHANNEL, TEST_BANK);
        uint8_t status = nand_cmd_read_status(TEST_CHANNEL, TEST_BANK);
        if (status != 0xE0)
        {
            log_print(info, "nand_cmd_zq_calibration_long fail: %#x\r\n", status);
            return RET_ERR;
        }
    }
    {
        nand_cmd_zq_calibration_short(TEST_CHANNEL, TEST_BANK);
        uint8_t status = nand_cmd_read_status(TEST_CHANNEL, TEST_BANK);
        if (status != 0xE0)
        {
            log_print(info, "nand_cmd_zq_calibration_short fail: %#x\r\n", status);
            return RET_ERR;
        }
    }
    return RET_OK;
}

uint32_t test_nand_cmd_get_set_feature(void)
{
    uint8_t nand_adr = 0x02;
    uint16_t wdt[CMD_FEATURE_NUM] = { 0x0707, 0, 0, 0 };

    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    nand_cmd_set_feature(TEST_CHANNEL, TEST_BANK, nand_adr, wdt);

    uint16_t rdt[CMD_FEATURE_NUM] = { 0 };
    nand_cmd_get_feature(TEST_CHANNEL, TEST_BANK, nand_adr, rdt);

    for (uint8_t i = 0; i < CMD_FEATURE_NUM; i++)
    {
        if (wdt[i] != rdt[i])
        {
            log_print(info, "nand_cmd_get_feature fail: %x %x\r\n", wdt[i], rdt[i]);
            return RET_ERR;
        }
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_cmd_read_unique_id(void)
{
    uint8_t i, j;
    uint8_t nand_adr = 0x00;
    uint8_t rdt[CMD_READ_UID_NUM] = { 0 };
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    uint32_t unique_id = nand_cmd_read_unique_id(TEST_CHANNEL, TEST_BANK, nand_adr, rdt);
    for (i = 0; i < CMD_READ_UID_NUM / 16; i++)
    {
        for (j = 0; j < 16; j++)
        {
            log_print(info, "0x%02x ", rdt[i * 16 + j]);
        }
        log_print(info, "\r\n");
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_cmd_odt_configure(void)
{
    uint8_t addr_list[] = { 0, 1 };
    uint16_t wdt_list[] = { 1, 2, 3, 4 };
    nand_cmd_odt_configure(TEST_CHANNEL, TEST_BANK, addr_list, ITEM_OF(addr_list), wdt_list, ITEM_OF(wdt_list));
    uint8_t status = nand_cmd_read_status(TEST_CHANNEL, TEST_BANK);
    if (status != 0xE0)
    {
        log_print(info, "nand_cmd_odt_configure fail: %#x\r\n", status);
        return RET_ERR;
    }

    return RET_OK;
}

uint32_t test_nand_cmd_read_id(void)
{
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    {
        /* Users can read six bytes of ID containing manufacturer code, device code and architecture information of the target
         * by command 90h followed by 00h address. The command register remains in Read ID mode until another command
         * is issued.
         * */
        uint8_t nand_adr = READ_ID_ADDR_0;
        uint8_t rdt[CMD_READ_ID_NUM] = { 0 };
        nand_cmd_read_id(TEST_CHANNEL, TEST_BANK, nand_adr, rdt);
        log_print(info, "Maker Code: %#x\r\n", rdt[0]);
        log_print(info, "Device Code: %#x\r\n", rdt[1]);
        log_print(info, "Number of LUN per Target, Cell Type, Etc: %#x\r\n", rdt[2]);
        log_print(info, "Page Size,etc: %#x\r\n", rdt[3]);
        log_print(info, "Plane Number,etc: %#x\r\n", rdt[4]);
        log_print(info, "Technology Code: %#x\r\n", rdt[5]);
    }
    {
        /* Toggle DDR Flash Memory also provides six bytes of JEDEC standard signature ID. Users can read the ID by
         * command 90h followed by 40h address. Any data returned after the six bytes of JEDEC standard signature is
         * considered reserved for future use.
         * */

        uint8_t nand_adr = READ_ID_ADDR_1;
        uint8_t rdt[CMD_READ_ID_NUM] = { 0 };
        nand_cmd_read_id(TEST_CHANNEL, TEST_BANK, nand_adr, rdt);
        log_print(info, "  J: %#x\r\n", rdt[0]);
        log_print(info, "  E: %#x\r\n", rdt[1]);
        log_print(info, "  D: %#x\r\n", rdt[2]);
        log_print(info, "  E: %#x\r\n", rdt[3]);
        log_print(info, "  C: %#x\r\n", rdt[4]);
        log_print(info, "DDR: %#x\r\n", rdt[5]);
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_cmd_read_parameter_page(void)
{
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    {
        uint8_t i, j;
        uint8_t nand_adr = 0x00;
        uint8_t rdt[CMD_READ_PARA_PAGE_NUM] = { 0 };
        nand_cmd_read_parameter_page(TEST_CHANNEL, TEST_BANK, nand_adr, rdt);
        for (i = 0; i < CMD_READ_PARA_PAGE_NUM / 16; i++)
        {
            for (j = 0; j < 16; j++)
            {
                log_print(info, "0x%02x ", rdt[i * 16 + j]);
            }
            log_print(info, "\r\n");
        }
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_cmd_read_status(void)
{
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    {
#if defined(NAND_BICS5)
        uint8_t addr_list[] = { 0, 1, 2 };
#elif defined(NAND_YX39070)
        uint8_t addr_list[] = { 0, 1, 2, 3 };
#endif
        uint8_t status = nand_cmd_read_status_enhanced(TEST_CHANNEL, TEST_BANK, addr_list, ITEM_OF(addr_list));
        log_print(info, "read status enhanced: %#x\r\n", status);
    }
#if defined(NAND_BICS5)
    {
        uint8_t status = nand_cmd_read_status_lun_previous(TEST_CHANNEL, TEST_BANK);
        log_print(info, "read status lun previous: %#x\r\n", status);
    }
    {
        uint8_t status = nand_cmd_read_status_lun(TEST_CHANNEL, TEST_BANK);
        log_print(info, "read status lun: %#x\r\n", status);
    }
#endif
    {
        uint8_t status = nand_cmd_read_status(TEST_CHANNEL, TEST_BANK);
        log_print(info, "read status: %#x\r\n", status);
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_cmd_get_set_feature_by_lun(void)
{
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    {
        uint8_t lun_adr = 0x0;
        uint8_t nand_adr = 0x02;

        uint16_t wdt[CMD_FEATURE_NUM] = { 0x0707, 0, 0, 0 };
        nand_cmd_set_feature_by_lun(TEST_CHANNEL, TEST_BANK, lun_adr, nand_adr, wdt);

        uint16_t rdt[CMD_FEATURE_NUM] = { 0 };
        nand_cmd_get_feature_by_lun(TEST_CHANNEL, TEST_BANK, lun_adr, nand_adr, rdt);

        for (uint8_t i = 0; i < CMD_FEATURE_NUM; i++)
        {
            if (wdt[i] != rdt[i])
            {
                log_print(info, "nand_cmd_get_feature_by_lun fail: %x %x\r\n", wdt[i], rdt[i]);
                return RET_ERR;
            }
        }
    }
    {
        uint8_t lun_adr = 0x1;
        uint8_t nand_adr = 0x02;

        uint16_t wdt[CMD_FEATURE_NUM] = { 0x0707, 0, 0, 0 };
        nand_cmd_set_feature_by_lun(TEST_CHANNEL, TEST_BANK, lun_adr, nand_adr, wdt);

        uint16_t rdt[CMD_FEATURE_NUM] = { 0 };
        nand_cmd_get_feature_by_lun(TEST_CHANNEL, TEST_BANK, lun_adr, nand_adr, rdt);

        for (uint8_t i = 0; i < CMD_FEATURE_NUM; i++)
        {
            if (wdt[i] != rdt[i])
            {
                log_print(info, "nand_cmd_get_feature_by_lun fail: %x %x\r\n", wdt[i], rdt[i]);
                return RET_ERR;
            }
        }
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_read_retry_tlc(void)
{
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    hal_nctl_erase_multi_blk(ERS_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK);
    UNIT_PARAM par0 = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, 0x0, 0 };
    test_base_one_unit_write(PROG_CPTR, par0);
    uint32_t ret = test_base_one_unit_read_check(PROG_CPTR, par0);
    if (ret != RET_OK)
    {
        return ret;
    }

    uint32_t t_p4k = get_p4k(TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, 0x0);
    uint16_t t_buff = hal_bmu_alloc_buff(SRAM_BUF, CPUA_GET);
    uint32_t t_buff_mem = hal_bmu_get_sram_mem_addr(t_buff);
    hal_nctl_read_4k_to_buff(READ_CPTR, t_buff, t_p4k);//ret = nctl_read_retry(READ_CPTR, t_buff, t_p4k);
    if (ret != RET_OK)
    {
        return ret;
    }
    for (uint16_t i = 0; i < TEST_READ_4K_SIZE; i += 4)
    {
        if ((*(volatile uint32_t *) (t_buff_mem + i)) != t_p4k + i)
        {
            hal_bmu_cpu_free_buff(t_buff);
            return RET_ERR;
        }
    }
    hal_bmu_cpu_free_buff(t_buff);

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_read_retry_slc(void)
{
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    hal_nctl_erase_multi_blk(ERS_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK);
    UNIT_PARAM par0 = { TEST_BLOCK, 0x3, TEST_BANK, TEST_CHANNEL, 0x0, 0 };
    test_base_one_unit_write(PROG_SLC_CPTR, par0);
    uint32_t ret = test_base_one_unit_read_check(PROG_SLC_CPTR, par0);
    if (ret != RET_OK)
    {
        return ret;
    }

    uint32_t t_p4k = get_p4k(TEST_BLOCK, 0x3, TEST_BANK, TEST_CHANNEL, 0x0);
    uint16_t t_buff = hal_bmu_alloc_buff(SRAM_BUF, CPUA_GET);
    uint32_t t_buff_mem = hal_bmu_get_sram_mem_addr(t_buff);
    hal_nctl_read_4k_to_buff(READ_SLC_CPTR, t_buff, t_p4k);//ret = nctl_read_retry(READ_SLC_CPTR, t_buff, t_p4k);
    if (ret != RET_OK)
    {
        return ret;
    }
    for (uint16_t i = 0; i < TEST_READ_4K_SIZE; i += 4)
    {
        if ((*(volatile uint32_t *) (t_buff_mem + i)) != t_p4k + i)
        {
            hal_bmu_cpu_free_buff(t_buff);
            return RET_ERR;
        }
    }
    hal_bmu_cpu_free_buff(t_buff);

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_single_erase(void)
{
    uint16_t rd_num = rand();
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    hal_nctl_erase_single_blk(ERS_SINGE_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK, 0x0);

    UNIT_PARAM par0 = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, 0x0, rd_num };
    test_base_one_unit_write(PROG_SINGE_CPTR, par0);
    UNIT_PARAM par1 = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, 0x1, rd_num };
    test_base_one_unit_write(PROG_SINGE_CPTR, par1);

    // single erase plane1, then read plane0 data
    hal_nctl_erase_single_blk(ERS_SINGE_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK, 0x1);

    if(test_base_one_unit_read_check(PROG_SINGE_CPTR, par0) == RET_ERR)
    {
        log_print(info, "one unit read check fail\r\n");
        return RET_ERR;
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_multi_cache_read_write(void)
{
    uint16_t rd_num = rand();
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    g_cmd_info_reg->reg_hacc_fun = (RW_HACC_FUN){
        .ctrl.reg_rcah_en = ENABLE,
        .ctrl.reg_wcah_en = ENABLE,
    };
    hal_nctl_erase_multi_blk(ERS_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK);

    UNIT_PARAM par0 = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, 0x0, rd_num };
    test_base_one_unit_write(PROG_CACHE_CPTR, par0);
    UNIT_PARAM par1 = { TEST_BLOCK, 0x3, TEST_BANK, TEST_CHANNEL, 0x0, rd_num };
    test_base_one_unit_write(PROG_CPTR, par1);

    if ((test_base_one_unit_read_check(PROG_CACHE_CPTR, par0) != RET_OK) ||
        (test_base_one_unit_read_check(PROG_CPTR, par1) != RET_OK))
    {
        log_print(info, "one unit read check fail\r\n");
        return RET_ERR;
    }

    g_cmd_info_reg->reg_hacc_fun.whole = DISABLE;
    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_single_cache_read_write(void)
{
    uint16_t rd_num = rand();
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    g_cmd_info_reg->reg_hacc_fun = (RW_HACC_FUN){
        .ctrl.reg_rcah_en = ENABLE,
        .ctrl.reg_wcah_en = ENABLE,
    };
    hal_nctl_erase_single_blk(ERS_SINGE_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK, 0x0);

    UNIT_PARAM par0 = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, 0x0, rd_num };
    test_base_one_unit_write(PROG_CACHE_SINGE_CPTR, par0);
    UNIT_PARAM par1 = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, 0x1, rd_num };
    test_base_one_unit_write(PROG_SINGE_CPTR, par1);

    if ((test_base_one_unit_read_check(PROG_CACHE_SINGE_CPTR, par0) != RET_OK) ||
        (test_base_one_unit_read_check(PROG_SINGE_CPTR, par1) != RET_OK))
    {
        log_print(info, "one unit read check fail\r\n");
        return RET_ERR;
    }

    g_cmd_info_reg->reg_hacc_fun.whole = DISABLE;
    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_multi_slc_cache_read_write(void)
{
    uint16_t rd_num = rand();
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    g_cmd_info_reg->reg_hacc_fun = (RW_HACC_FUN){
        .ctrl.reg_rcah_en = ENABLE,
        .ctrl.reg_wcah_en = ENABLE,
    };
    hal_nctl_erase_multi_blk(ERS_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK);

    UNIT_PARAM par0 = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, 0x0, rd_num };
    test_base_one_unit_write(PROG_CACHE_SLC_CPTR, par0);
    UNIT_PARAM par1 = { TEST_BLOCK, 0x3, TEST_BANK, TEST_CHANNEL, 0x0, rd_num };
    test_base_one_unit_write(PROG_SLC_CPTR, par1);

    if ((test_base_one_unit_read_check(PROG_CACHE_SLC_CPTR, par0) != RET_OK) ||
        (test_base_one_unit_read_check(PROG_SLC_CPTR, par1) != RET_OK))
    {
        log_print(info, "one unit read check fail\r\n");
        return RET_ERR;
    }

    g_cmd_info_reg->reg_hacc_fun.whole = DISABLE;
    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_single_slc_cache_read_write(void)
{
    uint16_t rd_num = rand();
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    g_cmd_info_reg->reg_hacc_fun = (RW_HACC_FUN){
        .ctrl.reg_rcah_en = ENABLE,
        .ctrl.reg_wcah_en = ENABLE,
    };
    hal_nctl_erase_multi_blk(ERS_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK);

    UNIT_PARAM par0 = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, 0x0, rd_num };
    test_base_one_unit_write(PROG_CACHE_SINGE_SLC_CPTR, par0);
    UNIT_PARAM par1 = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, 0x1, rd_num };
    test_base_one_unit_write(PROG_SINGE_SLC_CPTR, par1);

    if ((test_base_one_unit_read_check(PROG_CACHE_SINGE_SLC_CPTR, par0) != RET_OK) ||
        (test_base_one_unit_read_check(PROG_SINGE_SLC_CPTR, par1) != RET_OK))
    {
        log_print(info, "one unit read check fail\r\n");
        return RET_ERR;
    }

    g_cmd_info_reg->reg_hacc_fun.whole = DISABLE;
    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_single_slc_read_write(void)
{
    uint16_t rd_num = rand();
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    hal_nctl_erase_multi_blk(ERS_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK);

    for (uint8_t plane = 0; plane < PLANE_QNTY; plane++)
    {
        UNIT_PARAM par = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, plane, rd_num };
        test_base_one_unit_write(PROG_SINGE_SLC_CPTR, par);

        if (test_base_one_unit_read_check(PROG_SINGE_SLC_CPTR, par) != RET_OK)
        {
            log_print(info, "one unit read check fail\r\n");
            return RET_ERR;
        }
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_multi_slc_read_write(void)
{
    uint16_t rd_num = rand();
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    hal_nctl_erase_multi_blk(ERS_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK);

    UNIT_PARAM par = { TEST_BLOCK, 0, TEST_BANK, TEST_CHANNEL, 0x0, rd_num };
    test_base_one_unit_write(PROG_SLC_CPTR, par);

    if(test_base_one_unit_read_check(PROG_SLC_CPTR, par) == RET_ERR)
    {
        log_print(info, "one unit read check fail\r\n");
        return RET_ERR;
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_single_program_unit(void)
{
    uint16_t rd_num = rand();
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    hal_nctl_erase_multi_blk(ERS_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK);

    for (uint8_t plane = 0; plane < PLANE_QNTY; plane++)
    {
        UNIT_PARAM par = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, plane, rd_num };
        test_base_one_unit_write(PROG_SINGE_CPTR, par);

        if (test_base_one_unit_read_check(PROG_SINGE_CPTR, par) != RET_OK)
        {
            log_print(info, "one unit read check fail\r\n");
            return RET_ERR;
        }
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_multi_program_unit(void)
{
    uint16_t rd_num = rand();
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    hal_nctl_erase_multi_blk(ERS_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK);

    UNIT_PARAM par = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, 0x0, rd_num };
    test_base_one_unit_write(PROG_CPTR, par);

    if (test_base_one_unit_read_check(PROG_CPTR, par) != RET_OK)
    {
        log_print(info, "one unit read check fail\r\n");
        return RET_ERR;
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_multi_program_unit_with_auto_buffer(void)
{
    uint8_t i_mapu, frag;
    uint16_t i, read_ptr, page;
    uint16_t rd_num = rand();
    uint32_t read_buffer, t_p4k;

    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    hal_nctl_erase_multi_blk(ERS_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK);

    UNIT_PARAM par = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, 0x0, rd_num };
    test_base_one_unit_write(PROG_CPTR, par);

    for (i_mapu = 0; i_mapu < MAPU_NUM; i_mapu += 1)
    {
        page = (i_mapu & ~FRAG_MASK) >> FRAG_SHIFT;
        frag = (i_mapu & FRAG_MASK);
        t_p4k = get_p4k(TEST_BLOCK, page, TEST_BANK, TEST_CHANNEL, frag);

        hal_nctl_read_4k_to_buff(READ_CPTR, BPTR_INVLD, t_p4k);

        //uint64_t end_time = 0;//hardware_timer_get_system_tick_count() + CMD_MAX_WAIT_TIME;
        do {
            if (rb_errd_vld != 0)
            {
                rb_errd_vld = ERRD_CLR;
                return NCTL_READ_4K_CMD_ERR;
            }
            if (rb_smrd_vld == 0)
            {
                continue;
            }

            read_ptr = rw_smrd_bfp;
            rb_smrd_vld = SMRD_CLR;
            if (read_ptr == BPTR_INVLD)
            {
                return RET_ERR;
            }

            read_buffer = (read_ptr < SRAM_BUFF_POINTER_BASE) ? hal_bmu_get_dram_mem_addr(read_ptr) : hal_bmu_get_sram_mem_addr(read_ptr);
            for (i = 0; i < TEST_READ_4K_SIZE; i += 4)
            {
                if (*(volatile uint32_t *)(read_buffer + i) != (t_p4k + i + rd_num))
                {
                    //hal_bmu_cpu_free_buff(read_ptr);
                    //return RET_ERR;
                	break;
                }
            }
            hal_bmu_cpu_free_buff(read_ptr);
            break;
        } while (1);//hardware_timer_get_system_tick_count() < end_time);

        // log_print(debug, "[i_mapu:%d]read time out", i_mapu);
        // return NCTL_WAIT_LOOP_TIME_OUT;
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_read_max_read_count(void)
{
    uint16_t i, t_buff;
    uint32_t t_buff_mem, t_p4k;
    uint32_t ret1, ret2;
    uint16_t rd_num = rand();
    uint32_t read_count = 0, first_ecc_fail = 0;

    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    hal_nctl_erase_multi_blk(ERS_CPTR, TEST_BANK, TEST_CHANNEL, TEST_BLOCK);

    UNIT_PARAM par = { TEST_BLOCK, 0x0, TEST_BANK, TEST_CHANNEL, 0x0, rd_num };
    test_base_one_unit_write(PROG_CPTR, par);

    while (1)
    {
        t_p4k = get_p4k(TEST_BLOCK, 0, TEST_BANK, TEST_CHANNEL, 0);
        t_buff = hal_bmu_alloc_buff(SRAM_BUF, NAND_GET);
        t_buff_mem = hal_bmu_get_sram_mem_addr(t_buff);

        hal_nctl_read_4k_to_buff(READ_CPTR, t_buff, t_p4k);
        ret1 = nand_read_cmd_wait_and_check(TEST_CHANNEL, TEST_BANK);
        if (ret1 != RET_OK)
        {
            if (first_ecc_fail == 0)
            {
                first_ecc_fail = read_count + 1;
            }
            hal_nctl_read_4k_to_buff(READ_CPTR, t_buff, t_p4k);//ret1 = nctl_read_retry(READ_CPTR, t_buff, t_p4k);
        }
        for (i = 0; i < TEST_READ_4K_SIZE; i += 4)
        {
            if ((*(volatile uint32_t *) (t_buff_mem + i)) != t_p4k + i + par.rd_num)
            {
                ret2 = RET_ERR;
                break;
            }
        }
        hal_bmu_cpu_free_buff(t_buff);
        if ((ret1 != RET_OK) || (ret2 != RET_OK))
        {
            log_print(info, "break ret info: %#x, %d\r\n", ret1, ret2);
            break;
        }
        read_count++;
        if (read_count % 10000 == 0)
        {
            log_print(info, "read_count tag: %d, first_ecc_fail: %d\r\n", read_count, first_ecc_fail);
        }
    }

    log_print(info, "total read_count: %d, first_ecc_fail\r\n", read_count, first_ecc_fail);
    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_read_write_full_block(void)
{
    return test_base_factory_blk_write_read(TEST_BANK, TEST_CHANNEL, TEST_BLOCK);
}

uint32_t test_nand_read_write_full_disk(void)
{
    uint32_t success_cnt = 0;
    uint32_t failed_cnt = 0;
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    for(uint8_t chan = 0; chan < CHAN_QNTY; chan++)
    {
        for(uint8_t bank = 0; bank < BANK_QNTY; bank++)
        {
            for (uint16_t blk = 0; blk < BLK_QNTY; blk++)
            {
                if (test_base_factory_blk_write_read(bank, chan, blk) != 0)
                {
                    failed_cnt++;
                }
                else
                {
                    success_cnt++;
                }
            }
        }
    }

    log_print(info, "[rw_test]failed_cnt: %d, success_cnt: %d\r\n", failed_cnt, success_cnt);
    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_factory_bad_block_print_by_single(void)
{
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    // disable ldpc
    rb_ecc_en = DISABLE;
    rb_rdmz_en = DISABLE;

    for (uint8_t chan = 0; chan < CHAN_QNTY; chan++)
    {
        for (uint8_t bank = 0; bank < BANK_QNTY; bank++)
        {
            for (uint16_t blk = 0; blk < BLK_QNTY; blk++)
            {
                for (uint8_t plane = 0; plane < PLANE_QNTY; plane++)
                {
                    uint32_t ret = nand_factory_defective_check(blk, bank, chan, plane);
                    if (ret != RET_OK)
                    {
                        log_print(info, "[chan: %d, bank: %d, plane: %d, blk: %d] ret: %#x\r\n", chan, bank, plane, blk, ret);
                    }
                }
            }
        }
    }

    // enable ldpc
    rb_ecc_en = ENABLE;
    rb_rdmz_en = ENABLE;
    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint8_t test_base_get_one_unit_size(uint8_t cmd_ptr)
{
    if ((cmd_ptr == PROG_CPTR) || (cmd_ptr == PROG_CACHE_CPTR))
    {
        // wirte p4k num: 24
        return MAPU_NUM;
    }
    else if ((cmd_ptr == PROG_SINGE_CPTR) || (cmd_ptr == PROG_CACHE_SINGE_CPTR))
    {
        // wirte p4k num: 12
        return (MAPU_NUM / PLANE_QNTY);
    }
    else if ((cmd_ptr == PROG_SLC_CPTR) || (cmd_ptr == PROG_CACHE_SLC_CPTR))
    {
        // wirte p4k num: 8
        return FRAG_QNTY;
    }
    else if ((cmd_ptr == PROG_SINGE_SLC_CPTR) || (cmd_ptr == PROG_CACHE_SINGE_SLC_CPTR))
    {
        // wirte p4k num: 4
        return (FRAG_QNTY / PLANE_QNTY);
    }

    return 0;
}

uint32_t test_nand_basic_define_rw_test(void)
{
#if defined(NAND_YX39070)
#define SMDT_BASE 0x5  // sram data buffer base address
#define TEST_BLK  0x3
#define FRAG_MSK  0x1f
#define RBUF_OFS  96

    dwrd ld_bmu_base, ld_bmu_adr;
    byte lb_mapu_cnt = 0;

    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);

    // write data to bmu mem of program cmd
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        ld_bmu_base = SRAM_MEM_BASE + ((dwrd)lb_mapu_cnt << 12);
        for (ld_bmu_adr = 0; ld_bmu_adr < (4096); ld_bmu_adr = ld_bmu_adr + 4)
        {
            (*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) = ((0x2345 + (dwrd)lb_mapu_cnt) << 16) + ld_bmu_adr;
        }
    }

    // clear bmu mem of read cmd
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        ld_bmu_base = SRAM_MEM_BASE + (((dwrd)lb_mapu_cnt + RBUF_OFS) << 12);
        for (ld_bmu_adr = 0; ld_bmu_adr < (4096); ld_bmu_adr = ld_bmu_adr + 4)
        {
            (*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) = 0;
        }
    }

    // send erase cmd to nand ctl
    rd_cmd_padr = TEST_BLK << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT);
    rw_cmd_type = ERS_CPTR | LOW_PRI | CMD_END;

    // send program cmd to nand ctl
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        rb_cmd_qidx = QLNK_SMQ;
        rd_cmd_padr = (TEST_BLK << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) | (((dwrd)lb_mapu_cnt/FRAG_QNTY) << (BKCH_SHIFT + FRAG_SHIFT)) | ((dwrd)lb_mapu_cnt%FRAG_QNTY);
        rw_cmd_bptr = (SMDT_BASE << 9) | (word)lb_mapu_cnt;
        //rw_cmd_othr = 0xff;
		if (lb_mapu_cnt < (2 * FRAG_QNTY))
		{
			//rw_cmd_type = PROG_CPTR | LOW_PRI | (((lb_mapu_cnt == (FRAG_QNTY-1)) || (lb_mapu_cnt == (2*FRAG_QNTY-1))) ? CMD_END : 0);
			rw_cmd_type = PROG_CPTR | LOW_PRI | ((lb_mapu_cnt == (MAPU_NUM-1)) ? CMD_END : 0);
		}
		else
		{
			rw_cmd_type = PROG_U_CPTR | LOW_PRI | ((lb_mapu_cnt == (MAPU_NUM-1)) ? CMD_END : 0);
		}
    }

    // send read cmd to nand ctl
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        rb_cmd_qidx = QLNK_SMQ;
        rd_cmd_padr = (TEST_BLK << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) | (((dwrd)lb_mapu_cnt/FRAG_QNTY) << (BKCH_SHIFT + FRAG_SHIFT)) | ((dwrd)lb_mapu_cnt%FRAG_QNTY);
        rw_cmd_bptr = (SMDT_BASE << 9) | (RBUF_OFS + (word)lb_mapu_cnt);
        rw_cmd_type = READ_CPTR | LOW_PRI | ((lb_mapu_cnt == (MAPU_NUM - 1)) ? CMD_END : 0);
    }

    // check whether data ready
    lb_mapu_cnt = 0;
    while (lb_mapu_cnt < MAPU_NUM)
    {
        if (rb_smrd_vld)
        {
            rb_smrd_vld = SMRD_CLR;

            // check data from BMU
            ld_bmu_base = SRAM_MEM_BASE + (((dwrd)lb_mapu_cnt + RBUF_OFS) << 12);
            for (ld_bmu_adr = 0; ld_bmu_adr < (4096); ld_bmu_adr = ld_bmu_adr + 4)
            {
                if ((*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) != (((0x2345 + (dwrd)lb_mapu_cnt) << 16) + ld_bmu_adr))
                {
                    log_print(info, "data check fail: %x %x\r\n", (*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)), (((0x2345 + (dwrd)lb_mapu_cnt) << 16) + ld_bmu_adr));
                    return RET_ERR;
                }
            }
            lb_mapu_cnt++;
        }
    }
#else
#define SMDT_BASE 0x5  // sram data buffer base address
#define TEST_BLK  0x3
#define FRAG_MSK  0x7
#define RBUF_OFS  32

    dwrd ld_bmu_base, ld_bmu_adr;
    byte lb_mapu_cnt = 0;
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);

    // write data to bmu mem of program cmd
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        ld_bmu_base = SRAM_MEM_BASE + ((dwrd)lb_mapu_cnt << 12);
        for (ld_bmu_adr = 0; ld_bmu_adr < (4096); ld_bmu_adr = ld_bmu_adr + 4)
        {
            (*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) = ((0x2345 + (dwrd)lb_mapu_cnt) << 16) + ld_bmu_adr;
        }
    }

    // clear bmu mem of read cmd
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        ld_bmu_base = SRAM_MEM_BASE + (((dwrd)lb_mapu_cnt + RBUF_OFS) << 12);
        for (ld_bmu_adr = 0; ld_bmu_adr < (4096); ld_bmu_adr = ld_bmu_adr + 4)
        {
            (*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) = 0;
        }
    }

    // send erase cmd to nand ctl
    rd_cmd_padr = TEST_BLK << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT);
    rw_cmd_type = ERS_CPTR | LOW_PRI | CMD_END;

    // send program cmd to nand ctl
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        rb_cmd_qidx = QLNK_SMQ;
        rd_cmd_padr = (TEST_BLK << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) | (((dwrd)lb_mapu_cnt & ~FRAG_MSK) << BKCH_SHIFT) | ((dwrd)lb_mapu_cnt & FRAG_MSK);
        rw_cmd_bptr = (SMDT_BASE << 9) | (word)lb_mapu_cnt;
        rw_cmd_othr = 0xff;
        rw_cmd_type = PROG_CPTR | LOW_PRI | ((lb_mapu_cnt == (MAPU_NUM - 1)) ? CMD_END : 0);
    }

    // send read cmd to nand ctl
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        rb_cmd_qidx = QLNK_SMQ;
        rd_cmd_padr = (TEST_BLK << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) | (((dwrd)lb_mapu_cnt & ~FRAG_MSK) << BKCH_SHIFT) | ((dwrd)lb_mapu_cnt & FRAG_MSK);
        rw_cmd_bptr = (SMDT_BASE << 9) | RBUF_OFS | (word)lb_mapu_cnt;
        rw_cmd_type = READ_CPTR | LOW_PRI | ((lb_mapu_cnt == (MAPU_NUM - 1)) ? CMD_END : 0);
    }

    // check whether data ready
    lb_mapu_cnt = 0;
    while (lb_mapu_cnt < MAPU_NUM)
    {
        if (rb_smrd_vld)
        {
            rb_smrd_vld = SMRD_CLR;

            // check data from BMU
            ld_bmu_base = SRAM_MEM_BASE + (((dwrd)lb_mapu_cnt + RBUF_OFS) << 12);
            for (ld_bmu_adr = 0; ld_bmu_adr < (4096); ld_bmu_adr = ld_bmu_adr + 4)
            {
                if ((*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) != (((0x2345 + (dwrd)lb_mapu_cnt) << 16) + ld_bmu_adr))
                {
                    log_print(info, "data check fail: %x %x\r\n", (*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)), (((0x2345 + (dwrd)lb_mapu_cnt) << 16) + ld_bmu_adr));
                    return RET_ERR;
                }
            }
            lb_mapu_cnt++;
        }
    }
#endif
    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);

    return RET_OK;
}

uint32_t test_nand_basic_register_struct_rw_test(void)
{
#if defined(NAND_YX39070)
    dwrd ld_bmu_base, ld_bmu_adr;
    // clear bmu mem of program cmd
    byte lb_mapu_cnt = 0;
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);

    // write data to bmu mem of program cmd
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        ld_bmu_base = SRAM_MEM_BASE + ((dwrd)lb_mapu_cnt << 12);
        for (ld_bmu_adr = 0; ld_bmu_adr < (4096); ld_bmu_adr = ld_bmu_adr + 4)
        {
            (*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) = ((0x2345 + (dwrd)lb_mapu_cnt) << 16) + ld_bmu_adr;
        }
    }

    // clear bmu mem of read cmd
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        ld_bmu_base = SRAM_MEM_BASE + (((dwrd)lb_mapu_cnt + RBUF_OFS) << 12);
        for (ld_bmu_adr = 0; ld_bmu_adr < (4096); ld_bmu_adr = ld_bmu_adr + 4)
        {
            (*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) = 0;
        }
    }

    // send erase cmd to nand ctl
    g_cmd_info_reg->reg_cmd_padr = TEST_BLK << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT);
    g_cmd_info_reg->reg_cmd_type.whole = ERS_CPTR | LOW_PRI | CMD_END;

    // send program cmd to nand ctl
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        g_cmd_info_reg->reg_bmsg_que = QLNK_SMQ;
        g_cmd_info_reg->reg_cmd_padr = (TEST_BLK << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) | (((dwrd)lb_mapu_cnt/FRAG_QNTY) << (BKCH_SHIFT + FRAG_SHIFT)) | ((dwrd)lb_mapu_cnt%FRAG_QNTY);
        g_cmd_info_reg->reg_bmsg_bfp = (SMDT_BASE << 9) | (word)lb_mapu_cnt;
        g_cmd_info_reg->reg_bmsg_ltp = 0;
		if (lb_mapu_cnt < (2 * FRAG_QNTY))
		{
			g_cmd_info_reg->reg_cmd_type.whole = PROG_CPTR | LOW_PRI | ((lb_mapu_cnt == (MAPU_NUM-1)) ? CMD_END : 0);
		}
		else
		{
			g_cmd_info_reg->reg_cmd_type.whole = PROG_U_CPTR | LOW_PRI | ((lb_mapu_cnt == (MAPU_NUM-1)) ? CMD_END : 0);
		}
    }

    // send read cmd to nand ctl
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        g_cmd_info_reg->reg_bmsg_que = QLNK_SMQ;
        g_cmd_info_reg->reg_cmd_padr = (TEST_BLK << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) | (((dwrd)lb_mapu_cnt/FRAG_QNTY) << (BKCH_SHIFT + FRAG_SHIFT)) | ((dwrd)lb_mapu_cnt%FRAG_QNTY);
        g_cmd_info_reg->reg_bmsg_bfp = (SMDT_BASE << 9) | (RBUF_OFS + (word)lb_mapu_cnt);
        g_cmd_info_reg->reg_cmd_type.whole = READ_CPTR | LOW_PRI | ((lb_mapu_cnt == (MAPU_NUM - 1)) ? CMD_END : 0);
    }

    // check whether data ready
    lb_mapu_cnt = 0;
    while (lb_mapu_cnt < MAPU_NUM)
    {
        if (rb_smrd_vld)
        {
            rb_smrd_vld = SMRD_CLR;

            // check data from BMU
            ld_bmu_base = SRAM_MEM_BASE + (((dwrd)lb_mapu_cnt + RBUF_OFS) << 12);
            for (ld_bmu_adr = 0; ld_bmu_adr < (4096); ld_bmu_adr = ld_bmu_adr + 4)
            {
                if ((*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) != (((0x2345 + (dwrd)lb_mapu_cnt) << 16) + ld_bmu_adr))
                {
                    log_print(info, "data check fail: %x %x\r\n", (*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)), (((0x2345 + (dwrd)lb_mapu_cnt) << 16) + ld_bmu_adr));
                    return RET_ERR;
                }
            }
            lb_mapu_cnt++;
        }
    }
#else
    dwrd ld_bmu_base, ld_bmu_adr;
    // clear bmu mem of program cmd
    byte lb_mapu_cnt = 0;
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);

    // write data to bmu mem of program cmd
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        ld_bmu_base = SRAM_MEM_BASE + ((dwrd)lb_mapu_cnt << 12);
        for (ld_bmu_adr = 0; ld_bmu_adr < (4096); ld_bmu_adr = ld_bmu_adr + 4)
        {
            (*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) = ((0x2345 + (dwrd)lb_mapu_cnt) << 16) + ld_bmu_adr;
        }
    }

    // clear bmu mem of read cmd
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        ld_bmu_base = SRAM_MEM_BASE + (((dwrd)lb_mapu_cnt + RBUF_OFS) << 12);
        for (ld_bmu_adr = 0; ld_bmu_adr < (4096); ld_bmu_adr = ld_bmu_adr + 4)
        {
            (*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) = 0;
        }
    }

    // send erase cmd to nand ctl
    g_cmd_info_reg->reg_cmd_padr = TEST_BLK << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT);
    g_cmd_info_reg->reg_cmd_type.whole = ERS_CPTR | LOW_PRI | CMD_END;

    // send program cmd to nand ctl
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        g_cmd_info_reg->reg_bmsg_que = QLNK_SMQ;
        g_cmd_info_reg->reg_cmd_padr = (TEST_BLK << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) | (((dwrd)lb_mapu_cnt & ~FRAG_MSK) << BKCH_SHIFT) | ((dwrd)lb_mapu_cnt & FRAG_MSK);
        g_cmd_info_reg->reg_bmsg_bfp = (SMDT_BASE << 9) | (word)lb_mapu_cnt;
        g_cmd_info_reg->reg_bmsg_ltp = 0;
        g_cmd_info_reg->reg_cmd_type.whole = PROG_CPTR | LOW_PRI | ((lb_mapu_cnt == (MAPU_NUM - 1)) ? CMD_END : 0);
    }

    // send read cmd to nand ctl
    lb_mapu_cnt = 0;
    for (; lb_mapu_cnt < MAPU_NUM; lb_mapu_cnt = lb_mapu_cnt + 1)
    {
        g_cmd_info_reg->reg_bmsg_que = QLNK_SMQ;
        g_cmd_info_reg->reg_cmd_padr = (TEST_BLK << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) | (((dwrd)lb_mapu_cnt & ~FRAG_MSK) << BKCH_SHIFT) | ((dwrd)lb_mapu_cnt & FRAG_MSK);
        g_cmd_info_reg->reg_bmsg_bfp = (SMDT_BASE << 9) | RBUF_OFS | (word)lb_mapu_cnt;
        g_cmd_info_reg->reg_cmd_type.whole = READ_CPTR | LOW_PRI | ((lb_mapu_cnt == (MAPU_NUM - 1)) ? CMD_END : 0);
    }

    // check whether data ready
    lb_mapu_cnt = 0;
    while (lb_mapu_cnt < MAPU_NUM)
    {
        if (rb_smrd_vld)
        {
            rb_smrd_vld = SMRD_CLR;

            // check data from BMU
            ld_bmu_base = SRAM_MEM_BASE + (((dwrd)lb_mapu_cnt + RBUF_OFS) << 12);
            for (ld_bmu_adr = 0; ld_bmu_adr < (4096); ld_bmu_adr = ld_bmu_adr + 4)
            {
                if ((*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) != (((0x2345 + (dwrd)lb_mapu_cnt) << 16) + ld_bmu_adr))
                {
                    log_print(info, "data check fail: %x %x\r\n", (*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)), (((0x2345 + (dwrd)lb_mapu_cnt) << 16) + ld_bmu_adr));
                    return RET_ERR;
                }
            }
            lb_mapu_cnt++;
        }
    }
#endif
    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

void test_base_write_wait_for_read(void)
{
	uint32_t i = 0;
	for(i = 0; i < 0xfffffff; i++)
		;
}

uint32_t test_base_one_unit_write(uint8_t cmd_ptr, UNIT_PARAM par)
{
    // get loop size range by cmd_ptr
    uint8_t unit_size = test_base_get_one_unit_size(cmd_ptr);
    uint8_t i_mapu = 0;
    uint8_t frag = 0;
    uint8_t p_end_size = unit_size;
    uint16_t page = 0;
    uint16_t i;
    for (i_mapu = 0; i_mapu < unit_size; i_mapu++)
    {
        frag = 0;
        page = 0;
        if ((cmd_ptr == PROG_CPTR) || (cmd_ptr == PROG_CACHE_CPTR))
        {
            // wirte p4k num: 24
#if defined(NAND_YX39070)
            page = par.first_page + (i_mapu / FRAG_QNTY);
            frag = i_mapu % FRAG_QNTY;
            u_page_cptr = PROG_U_CPTR;
#else
            page = par.first_page + ((i_mapu & ~FRAG_MASK) >> FRAG_SHIFT);
            frag = i_mapu & FRAG_MASK;
#endif
            p_end_size = FRAG_QNTY;
        }
        else if ((cmd_ptr == PROG_SINGE_CPTR) || (cmd_ptr == PROG_CACHE_SINGE_CPTR))
        {
            // wirte p4k num: 12
#if defined(NAND_YX39070)
            page = par.first_page + (i_mapu  / (FRAG_QNTY / PLANE_QNTY));
            frag = par.plane * (FRAG_QNTY / PLANE_QNTY) + (i_mapu % (FRAG_QNTY / PLANE_QNTY));
            u_page_cptr = PROG_SINGE_UPPER_CPTR;
#else
            page = par.first_page + (((i_mapu * PLANE_QNTY) & ~FRAG_MASK) >> FRAG_SHIFT);
            frag = par.plane * (FRAG_QNTY / PLANE_QNTY) + (i_mapu & (FRAG_MASK >> (PLANE_QNTY / 2)));
#endif
            p_end_size = FRAG_QNTY / PLANE_QNTY; //for single page, must set end tag
        }
        else if ((cmd_ptr == PROG_SLC_CPTR) || (cmd_ptr == PROG_CACHE_SLC_CPTR))
        {
            // wirte p4k num: 8
            page = par.first_page;
            frag = i_mapu;
            p_end_size = FRAG_QNTY;
        }
        else if ((cmd_ptr == PROG_SINGE_SLC_CPTR) || (cmd_ptr == PROG_CACHE_SINGE_SLC_CPTR))
        {
            // wirte p4k num: 4
            page = par.first_page;
            frag = par.plane * (FRAG_QNTY / PLANE_QNTY) + i_mapu;
            p_end_size = FRAG_QNTY / PLANE_QNTY; //for single page, must set end tag
        }

        uint32_t t_p4k = get_p4k(par.blk, page, par.bank, par.ch, frag);
        uint16_t t_buff = hal_bmu_alloc_buff(SRAM_BUF, CPUA_GET);
        uint32_t t_buff_mem = hal_bmu_get_sram_mem_addr(t_buff);
        for (i = 0; i < TEST_WRITE_4K_SIZE; i += 4)
        {
            (*(volatile uint32_t *)(t_buff_mem + i)) = t_p4k + i + par.rd_num;
        }
#if defined(NAND_YX39070)
		if (i_mapu < (2 * p_end_size))
		{
        	hal_nctl_program_buff_4k(cmd_ptr, t_buff, t_p4k, 0);
		}
		else
		{
			if ((i_mapu % p_end_size) == (p_end_size - 1))
			{
				for(uint32_t j = 0; j < 0x10000; j++)
            	;
				hal_nctl_program_buff_4k(u_page_cptr, t_buff, t_p4k, 1);
			}
			else
				hal_nctl_program_buff_4k(u_page_cptr, t_buff, t_p4k, 0);
		}
#else
        if ((i_mapu % p_end_size) == (p_end_size - 1))
        {
            for(uint32_t j = 0; j < 0x10000; j++)
            	;
        	hal_nctl_program_buff_4k(cmd_ptr, t_buff, t_p4k, 1);
        }
        else
        {
        	hal_nctl_program_buff_4k(cmd_ptr, t_buff, t_p4k, 0);
        }
#endif
	}

    return RET_OK;
}

uint32_t test_base_one_unit_read_check(uint8_t cmd_ptr, UNIT_PARAM par)
{
    // get loop size range by cmd_ptr
    uint8_t unit_size = test_base_get_one_unit_size(cmd_ptr);
    uint8_t frag = 0;
    uint8_t read_ptr = 0;
    uint16_t page = 0;
    uint16_t i;
    //test_base_write_wait_for_read();
    for (uint8_t i_mapu = 0; i_mapu < unit_size; i_mapu++)
    {
        frag = 0;
        read_ptr = 0;
        page = 0;
        if ((cmd_ptr == PROG_CPTR) || (cmd_ptr == PROG_CACHE_CPTR))
        {
            // wirte p4k num: 24
#if defined(NAND_YX39070)
            page = par.first_page + (i_mapu/FRAG_QNTY);
            frag = i_mapu%FRAG_QNTY;
#else
            page = par.first_page + ((i_mapu & ~FRAG_MASK) >> FRAG_SHIFT);
            frag = i_mapu & FRAG_MASK;
#endif
            read_ptr = READ_CPTR;
        }
        else if ((cmd_ptr == PROG_SINGE_CPTR) || (cmd_ptr == PROG_CACHE_SINGE_CPTR))
        {
            // wirte p4k num: 12
            page = par.first_page + (((i_mapu * PLANE_QNTY) & ~FRAG_MASK) >> FRAG_SHIFT);
            frag = par.plane * (FRAG_QNTY / PLANE_QNTY) + (i_mapu & (FRAG_MASK >> (PLANE_QNTY / 2)));
            read_ptr = READ_CPTR;
        }
        else if ((cmd_ptr == PROG_SLC_CPTR) || (cmd_ptr == PROG_CACHE_SLC_CPTR))
        {
            // wirte p4k num: 8
            page = par.first_page;
            frag = i_mapu;
            read_ptr = READ_SLC_CPTR;
        }
        else if ((cmd_ptr == PROG_SINGE_SLC_CPTR) || (cmd_ptr == PROG_CACHE_SINGE_SLC_CPTR))
        {
            // wirte p4k num: 4
            page = par.first_page;
            frag = par.plane * (FRAG_QNTY / PLANE_QNTY) + i_mapu;
            read_ptr = READ_SLC_CPTR;
        }

        uint32_t t_p4k = get_p4k(par.blk, page, par.bank, par.ch, frag);
        uint16_t t_buff = hal_bmu_alloc_buff(SRAM_BUF, CPUA_GET);
        uint32_t t_buff_mem = hal_bmu_get_sram_mem_addr(t_buff);
        hal_nctl_read_4k_to_buff(read_ptr, t_buff, t_p4k);
        if(hal_bmu_wait_read_buff_vld() == RET_ERR)
        {
            log_print(info, "read 4k to buff fail\r\n");
            hal_bmu_clear_errq();
            return RET_ERR;
        }
        hal_bmu_clear_smrdq();
        for (i = 0; i < TEST_READ_4K_SIZE; i += 4)
        {
            if ((*(volatile uint32_t *)(t_buff_mem + i)) != t_p4k + i + par.rd_num)
            {
                log_print(info, "data check fail: %x %x\r\n", (*(volatile uint32_t *)(t_buff_mem + i)), t_p4k + i + par.rd_num);
                hal_bmu_cpu_free_buff(t_buff);
                return RET_ERR;
            }
        }
        hal_bmu_cpu_free_buff(t_buff);
    }

    return RET_OK;
}

uint32_t test_base_func_blk_write(uint8_t bank, uint8_t chan, uint16_t blk)
{
    uint8_t frag;
    uint16_t i_pu, i_mapu, i, page, t_buff;
    uint32_t t_p4k, t_buff_mem;
    uint32_t ret;

    // page loop; tlc: one wl has three pages
    for (i_pu = 0; i_pu < PU_QNTY; i_pu += 1)
    {
        // program unit loop; total num: 24
        for (i_mapu = 0; i_mapu < MAPU_NUM; i_mapu += 1)
        {
            // when i_mapu every full one FRAG_MASK: page++
#if defined(NAND_YX39070)
            page = (i_pu * 3) + (i_mapu/FRAG_QNTY);
            frag = i_mapu%FRAG_QNTY;
#else
            page = (i_pu * FRAG_SHIFT) + ((i_mapu & ~FRAG_MASK) >> FRAG_SHIFT);
            // frag range: [0, FRAG_MASK]
            frag = i_mapu & FRAG_MASK;
#endif

            t_p4k = get_p4k(blk, page, bank, chan, frag);
            // bmu: don't free by yourself; when program unit write, this will free by bmu
            t_buff = hal_bmu_alloc_buff(SRAM_BUF, CPUA_GET);
            t_buff_mem = hal_bmu_get_sram_mem_addr(t_buff);

            // prepare the pattern and program
            for (i = 0; i < TEST_WRITE_4K_SIZE; i += 4)
            {
                (*(volatile uint32_t *)(t_buff_mem + i)) = (t_p4k + i);
            }

#if defined(NAND_YX39070)
			if(i_mapu < (2*FRAG_QNTY))
			{
	        	hal_nctl_program_buff_4k(PROG_CPTR, t_buff, t_p4k, 0);
			}
			else
			{
				if(i_mapu == (MAPU_NUM - 1))
				{
					for(uint32_t j = 0; j < 0x10000; j++)
	            	;
					hal_nctl_program_buff_4k(PROG_U_CPTR, t_buff, t_p4k, 1);
				}
				else
					hal_nctl_program_buff_4k(PROG_U_CPTR, t_buff, t_p4k, 0);
			}
#else
            hal_nctl_program_buff_4k(PROG_CPTR, t_buff, t_p4k, 0);
#endif
        }
        ret = nand_write_cmd_wait_and_check(chan, bank);
        if (ret != RET_OK)
        {
            log_print(info, "[i_pu: %d] program err: %#x\r\n", i_pu, ret);
            return ret;
        }
    }

    return RET_OK;
}

uint32_t test_base_func_blk_read(uint8_t bank, uint8_t chan, uint16_t blk)
{
    uint8_t frag;
    uint16_t i_pu, i_mapu, i, page, t_buff;
    uint32_t t_p4k, t_buff_mem;
    uint32_t ret;

    for (i_pu = 0; i_pu < PU_QNTY; i_pu += 1)
    {
        for (i_mapu = 0; i_mapu < MAPU_NUM; i_mapu += 1)
        {
            // when i_mapu every full one FRAG_MASK: page++
#if defined(NAND_YX39070)
			page = (i_pu * 3) + (i_mapu/FRAG_QNTY);
			frag = i_mapu%FRAG_QNTY;
#else
            page = (i_pu * FRAG_SHIFT) + ((i_mapu & ~FRAG_MASK) >> FRAG_SHIFT);
            // frag range: [0, FRAG_MASK]
            frag = i_mapu & FRAG_MASK;
#endif
            t_p4k = get_p4k(blk, page, bank, chan, frag);
            t_buff = hal_bmu_alloc_buff(SRAM_BUF, CPUA_GET);
            t_buff_mem = hal_bmu_get_sram_mem_addr(t_buff);

            hal_nctl_read_4k_to_buff(READ_CPTR, t_buff, t_p4k);
            ret = nand_read_cmd_wait_and_check(chan, bank);
            if (ret != RET_OK)
            {
                log_print(info, "[page: %d, frag: %d]\r\n", page, frag);
                return ret;
            }

            for (i = 0; i < TEST_READ_4K_SIZE; i += 4)
            {
                if ((*(volatile uint32_t *)(t_buff_mem + i)) != (t_p4k + i))
                {
                    log_print(info, "block read check fail: %x %x\r\n", (*(volatile uint32_t *)(t_buff_mem + i)), (t_p4k + i));
                    hal_bmu_cpu_free_buff(t_buff);
                    return RET_ERR;
                }
            }
            hal_bmu_cpu_free_buff(t_buff);
        }
    }

    return RET_OK;
}

uint32_t test_base_factory_blk_write_read(uint8_t bank, uint8_t chan, uint16_t blk)
{
    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    hal_nctl_erase_multi_blk(ERS_CPTR, bank, chan, blk);
    uint32_t ret = nand_erase_cmd_wait_and_check(chan, bank);
    if (ret != RET_OK)
    {
        // bad block(skip)
        log_print(info, "[ce: %d, chan: %d, blk: %d] bad block(skip): %#x\r\n", bank, chan, blk, ret);
        return RET_OK;
    }

    ret = test_base_func_blk_write(bank, chan, blk);
    if (ret != RET_OK)
    {
        log_print(info, "[ce: %d, chan: %d, blk: %d] blk write err: %#x\r\n", bank, chan, blk, ret);
        return RET_ERR;
    }
    ret = test_base_func_blk_read(bank, chan, blk);
    if (ret != RET_OK)
    {
        log_print(info, "[ce: %d, chan: %d, blk: %d] blk read err: %#x\r\n", bank, chan, blk, ret);
        return RET_ERR;
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t update_tlc_p4k_in_one_multi_blk(PHY_ADR *lp_curr_padr)
{
    if (lp_curr_padr->sb_head_frag != (FRAG_QNTY - 1))
    {
        lp_curr_padr->sb_head_frag++;
        return RET_OK;
    }

    lp_curr_padr->sb_head_frag = 0x0;
    if (lp_curr_padr->sb_head_wcnt != lp_curr_padr->sb_head_wend)
    {
        lp_curr_padr->sb_head_wcnt++;
        lp_curr_padr->sw_head_page++;
        return RET_OK;
    }

    lp_curr_padr->sb_head_wcnt = 0x0;
    if (lp_curr_padr->sw_head_page != (PAGE_QNTY - 1))
    {
        lp_curr_padr->sw_head_page++;
        return RET_OK;
    }

    return RET_ERR;
}

extern DBL_TBL gs_dbl_tbl[RBLK_QNTY];
extern word gw_fblk_str;
extern word gw_fblk_end;
extern word gw_fblk_num;
uint16_t test_gw_bblk_str = DBLK_INVLD;
uint16_t test_gw_bblk_end = DBLK_INVLD;
uint16_t test_gw_bblk_num = 0;
uint64_t test_bad_die_position[RBLK_QNTY];

uint32_t test_nand_erase_interleave(void)
{
#define TEST_BLOCK_NUM      20
    uint8_t lb_bank, lb_chan;
    uint16_t blk_ptr;

    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    for (blk_ptr = 0; blk_ptr < TEST_BLOCK_NUM; blk_ptr = blk_ptr + 1)
    {
    	log_print(info, "Erase block %d\r\n", blk_ptr);
        //erase all blocks
        for (lb_bank = 0; lb_bank < BANK_QNTY; lb_bank = lb_bank + 1)
        {
            for (lb_chan = 0; lb_chan < CHAN_QNTY; lb_chan = lb_chan + 1)
            {
                hal_nctl_erase_multi_blk(ERS_CPTR, lb_bank, lb_chan, blk_ptr);
            }
        }

        for (lb_bank = 0; lb_bank < BANK_QNTY; lb_bank = lb_bank + 1)
        {
            for (lb_chan = 0; lb_chan < CHAN_QNTY; lb_chan = lb_chan + 1)
            {
                uint32_t ret = nand_erase_cmd_wait_and_check(lb_chan, lb_bank);
                if (ret != RET_OK)
                {  // bad block(skip)
                    log_print(info, "[bank: %d, chan: %d, blk: %d] bad block(skip): %#x\r\n", lb_bank, lb_chan, blk_ptr, ret);
                    test_bad_die_position[blk_ptr] |= (1ull << (lb_bank * CHAN_QNTY + lb_chan));
                    //gs_dbl_tbl[blk_ptr].bad_die_position |= (1ull << (lb_bank * CHAN_QNTY + lb_chan));
                }
            }
        }
    }

    for (blk_ptr = 0; blk_ptr < TEST_BLOCK_NUM; blk_ptr = blk_ptr + 1)
    {
        if (test_bad_die_position[blk_ptr] == 0 /*gs_dbl_tbl[blk_ptr].bad_die_position == 0*/)
        {
            if(gw_fblk_str == DBLK_INVLD)
            {
                gw_fblk_str = blk_ptr;
            }
            else
            {
                gs_dbl_tbl[gw_fblk_end].wd.w_next_ptr = blk_ptr;
                gs_dbl_tbl[blk_ptr].wd.w_last_ptr = gw_fblk_end;
            }
            gw_fblk_end = blk_ptr;
            gs_dbl_tbl[gw_fblk_end].wd.w_next_ptr = DBLK_INVLD;
            gw_fblk_num++;
        }
        else
        {
            //check whether the first bad block is, mark the start of list
            if(test_gw_bblk_str == DBLK_INVLD)
            {
                test_gw_bblk_str = blk_ptr;
            }
            else
            {
                gs_dbl_tbl[test_gw_bblk_end].wd.w_next_ptr = blk_ptr;
            }
            //mark the end of list
            test_gw_bblk_end = blk_ptr;
            gs_dbl_tbl[test_gw_bblk_end].wd.w_next_ptr = DBLK_INVLD;
            test_gw_bblk_num++;
        }
    }

    if (gw_fblk_num + test_gw_bblk_num != TEST_BLOCK_NUM)
    {
        log_print(info, "\r\n");
        return RET_ERR;
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_read_write_interleave(void)
{
#define TEST_BLOCK_NUM      20
#define TEST_PAGE_NUM       3
    uint8_t lb_bank, lb_chan;
    uint16_t blk_ptr, page_ptr;

    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    for (blk_ptr = gw_fblk_str; blk_ptr < TEST_BLOCK_NUM; blk_ptr = gs_dbl_tbl[blk_ptr].wd.w_next_ptr)
    {
    	log_print(info, "block: %d\r\n", blk_ptr);
        for (page_ptr = 0; page_ptr < TEST_PAGE_NUM; page_ptr = page_ptr + 3)
        {
            for (lb_bank = 0; lb_bank < BANK_QNTY; lb_bank = lb_bank + 1)
            {
                for (lb_chan = 0; lb_chan < CHAN_QNTY; lb_chan = lb_chan + 1)
                {
                    UNIT_PARAM par = { blk_ptr, page_ptr, lb_bank, lb_chan, 0x0, 0 };
                    test_base_one_unit_write(PROG_CPTR, par);
                    uint32_t ret = test_base_one_unit_read_check(PROG_CPTR, par);
                    if (ret != RET_OK)
                    {
                        log_print(info, "block read check fail, chan %d, bank %d, page: %d\r\n", lb_chan, lb_bank, page_ptr);
                        return ret;
                    }
                }
            }
        }
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_nand_multi_write_read_error_handle()
{
#define TEST_BLOCK_NUM      20
#define TEST_PAGE_NUM       3
#define TOTAL_DIE_NUM       (BANK_QNTY * CHAN_QNTY)
#define PRO_UNIT_NUM        24

    uint8_t lb_bank, lb_chan, i_mapu, frag_ptr, die_index;
    uint16_t blk_ptr, page_ptr, t_buff[TOTAL_DIE_NUM][PRO_UNIT_NUM], i;
    uint32_t t_p4k, t_buff_mem;
    uint64_t error_bit;

    log_print(info, "\r\n--->[TEST_NAND]---> %s START\r\n", __func__);
    for (blk_ptr = test_gw_bblk_str; blk_ptr < TEST_BLOCK_NUM; blk_ptr = gs_dbl_tbl[blk_ptr].wd.w_next_ptr)
    {
        for (page_ptr = 0; page_ptr < TEST_PAGE_NUM; page_ptr = page_ptr + 3)
        {
            for (lb_bank = 0; lb_bank < BANK_QNTY; lb_bank = lb_bank + 1)
            {
                for (lb_chan = 0; lb_chan < CHAN_QNTY; lb_chan = lb_chan + 1)
                {
                    for(i_mapu = 0; i_mapu < MAPU_NUM; i_mapu++)
                    {
#if defined(NAND_YX39070)
						uint16_t temp_page_ptr = page_ptr + (i_mapu/FRAG_QNTY);
						die_index = lb_chan + lb_bank * BANK_QNTY;
						frag_ptr = i_mapu%FRAG_QNTY;
#else
                        uint16_t temp_page_ptr = page_ptr + (i_mapu >> 3);
                        die_index = lb_chan + lb_bank * BANK_QNTY;
                        frag_ptr = i_mapu & FRAG_MASK;
#endif
                        t_p4k = ((dwrd)blk_ptr << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) |
                                ((dwrd)temp_page_ptr << (BKCH_SHIFT + FRAG_SHIFT)) |
                                ((dwrd)lb_bank << (CHAN_SHIFT + FRAG_SHIFT)) |
                                ((dwrd)lb_chan << FRAG_SHIFT) |
                                ((dwrd)frag_ptr);
                        t_buff[die_index][i_mapu] = hal_bmu_alloc_buff(SRAM_BUF, HOST_GET);
                        t_buff_mem = hal_bmu_get_sram_mem_addr(t_buff[die_index][i_mapu]);
                        for (i = 0; i < TEST_WRITE_4K_SIZE; i += 4)
                        {
                            (*(volatile uint32_t *)(t_buff_mem + i)) = t_p4k + i;
                        }
#if defined(NAND_YX39070)
						if(i_mapu < (2*FRAG_QNTY))
						{
							hal_nctl_program_buff_4k(PROG_CPTR, t_buff[die_index][i_mapu], t_p4k, 0);
						}
						else
						{
							if(i_mapu == (MAPU_NUM - 1))
							{
								hal_nctl_program_buff_4k(PROG_U_CPTR, t_buff[die_index][i_mapu], t_p4k, 1);
							}
							else
							{
								hal_nctl_program_buff_4k(PROG_U_CPTR, t_buff[die_index][i_mapu], t_p4k, 0);
							}
						}
#else
                        hal_nctl_program_buff_4k(PROG_CPTR, t_buff[die_index][i_mapu], t_p4k, 0);
#endif
					}
                }
            }

            for (lb_bank = 0; lb_bank < BANK_QNTY; lb_bank = lb_bank + 1)
            {
                for (lb_chan = 0; lb_chan < CHAN_QNTY; lb_chan = lb_chan + 1)
                {
                    die_index = lb_chan + lb_bank * BANK_QNTY;
                    g_nand_set_reg->reg_index = lb_chan;

                    while(g_channel_set_reg->ro_cmd_sts.whole != CMD_STS_ALL_FIN);
                    while(g_channel_set_reg->ro_que_rdy.whole != QUE_RDY_ALL_READY);
                    while(g_channel_set_reg->que_abt_st != QABT_FSM_IDLE_ST);
                    while((g_channel_set_reg->status_rdy & (1 << lb_bank)) == 0);

                    if (g_channel_set_reg->status_fail & (1 << lb_bank))
                    {
                        error_bit = 1ull << die_index;
                        log_print(info, "write error, page: %d, blk: %d, bank: %d, ch: %d\r\n", page_ptr, blk_ptr, lb_bank, lb_chan);
                        //clear erase fail status
                        g_channel_set_reg->status_fail = g_channel_set_reg->status_fail & (1 << lb_bank);
                        if (!(error_bit & test_bad_die_position[blk_ptr]))//!(error_bit & gs_dbl_tbl[blk_ptr].bad_die_position))
                        {
                            log_print(info, "block write fail\r\n");
                            return RET_ERR;
                        }
                    }
                }
            }

            for (lb_bank = 0; lb_bank < BANK_QNTY; lb_bank = lb_bank + 1)
            {
                for (lb_chan = 0; lb_chan < CHAN_QNTY; lb_chan = lb_chan + 1)
                {
                    UNIT_PARAM par = { blk_ptr, page_ptr, lb_bank, lb_chan, 0x0, 0 };
                    int32_t ret = test_base_one_unit_read_check(PROG_CPTR, par);
                    die_index = lb_chan + lb_bank * BANK_QNTY;
                    if (ret != RET_OK)
                    {
                        error_bit = 1ull << die_index;
                        if (!(error_bit & test_bad_die_position[blk_ptr]))//!(error_bit & gs_dbl_tbl[blk_ptr].bad_die_position))
                        {
                            log_print(info, "block read check fail\r\n");
                            return RET_ERR;
                        }
                    }
                }
            }
        }
    }

    log_print(info, "<---[TEST_NAND]<--- %s PASS\r\n", __func__);
    return RET_OK;
}

uint32_t test_ntcl_training_flow(void)
{
    rb_ecc_en = DISABLE;
    rb_rdmz_en = DISABLE;

    int32_t ret = nand_data_training(TEST_CHANNEL, TEST_BANK);

    rb_ecc_en = ENABLE;
    rb_rdmz_en = ENABLE;
    return ret;
}
