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

#define PU_QNTY            (PAGE_QNTY / 3)
#if defined(NAND_YX39070)
#define MAPU_NUM           72
#define META_NUM           32
#else
#define MAPU_NUM           24
#define META_NUM           32
#endif
#define META_CRC_LEN       4
#define TEST_WRITE_4K_SIZE (4096 + META_NUM)
#define TEST_READ_4K_SIZE  (TEST_WRITE_4K_SIZE - META_CRC_LEN)  // the 4K read(last 4byte is use to auto CRC check)

#define CMD_STS_ALL_FIN     0x8
#define CMDQ_VLD_ALL_INVLD  0x0
#define QUE_WPTR_ALL_EMPTY  0x0
#define QUE_RDY_ALL_READY   0xffffffff
#define QABT_FSM_IDLE_ST    0x0
#define REG_RDS_DAT_READY   (1 << 6)
#define REG_RDS_DAT_PE_FAIL (1 << 0)


uint32_t nand_erase_cmd_wait_and_check(uint8_t chan, uint8_t bank)
{
    g_nand_set_reg->reg_index = chan;
    //uint64_t end_time = hardware_timer_get_system_tick_count() + CMD_MAX_WAIT_TIME;
    do {
        if ((g_channel_set_reg->ro_cmd_sts.whole != CMD_STS_ALL_FIN) ||
            (g_channel_set_reg->ro_cmdq_vld.whole != CMDQ_VLD_ALL_INVLD) ||
            (g_channel_set_reg->ro_que_rdy.whole != QUE_RDY_ALL_READY) ||
            (g_channel_set_reg->que_abt_st != QABT_FSM_IDLE_ST) ||
            ((g_channel_set_reg->status_rdy & (1 << bank)) == 0))
        {
            continue;
        }

        if(g_channel_set_reg->status_fail & (1 << bank))
        {
            g_channel_set_reg->status_fail = g_channel_set_reg->status_fail & (1 << bank);
            return NCTL_CHECK_STATUS_BAD_BLOCK;
        }
        return RET_OK;
    } while (1);//hardware_timer_get_system_tick_count() < end_time);
    //return NCTL_WAIT_LOOP_TIME_OUT;
}

uint32_t nand_write_cmd_wait_and_check(uint8_t chan, uint8_t bank)
{
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_cmd_lunce = bank;

    //uint64_t end_time = hardware_timer_get_system_tick_count() + CMD_MAX_WAIT_TIME;
    do {
        if ((g_channel_set_reg->ro_cmd_sts.whole != CMD_STS_ALL_FIN) ||
            (g_channel_set_reg->ro_que_rdy.whole != QUE_RDY_ALL_READY) ||
            (g_channel_set_reg->que_abt_st != QABT_FSM_IDLE_ST) ||
            (g_channel_set_reg->status_rdy & (1 << bank)) == 0) {
            continue;
        }

        if (g_channel_set_reg->ro_die_sts.ctrl.fail != 0)
        {
            return NCTL_CHECK_STATUS_NAND_DIE_PE_ERR;
        }
        else if (g_nand_set_reg->sts_fail != 0)
        {
            return NCTL_CHECK_STATUS_CHANNEL_MAP_ERR;
        }
        else if (g_nand_set_reg->reg_fail_info != 0)
        {
            return NCTL_CHECK_STATUS_NAND_PE_ERR;
        }
        else if (g_nand_set_reg->reg_sts_mask != 0)
        {
            return NCTL_CHECK_STATUS_MASK_PE_ERR;
        }
        else if ((g_channel_set_reg->reg_rds_dat & REG_RDS_DAT_PE_FAIL) != 0)
        {
            return NCTL_CHECK_STATUS_NAND_STATUS_ERR;
        }
        return RET_OK;
    } while (1);//hardware_timer_get_system_tick_count() < end_time);
    //return NCTL_WAIT_LOOP_TIME_OUT;
}

uint32_t nand_read_cmd_wait_and_check(uint8_t chan, uint8_t bank)
{
    //g_nand_set_reg->reg_index = chan;
    //g_channel_set_reg->reg_cmd_lunce = bank;
    rb_index = chan;
    rb_cmd_lunce = bank;

    //uint64_t end_time = hardware_timer_get_system_tick_count() + CMD_MAX_WAIT_TIME;
    do {
        if (rb_errd_vld != 0)
        {
            rb_errd_vld = ERRD_CLR;
            return RET_ERR;
        }
        if ((g_channel_set_reg->ro_que_rdy.whole != QUE_RDY_ALL_READY) ||
            (g_channel_set_reg->que_abt_st != QABT_FSM_IDLE_ST) ||
            (g_channel_set_reg->status_rdy & (1 << bank)) == 0 ||
            (g_channel_set_reg->reg_rds_dat & REG_RDS_DAT_READY) == 0)
        {
            continue;
        }
        if (rb_smrd_vld == 0)
        {
            continue;
        }
        rb_smrd_vld = SMRD_CLR;
        return RET_OK;
    } while (1);//hardware_timer_get_system_tick_count() < end_time);
    //return NCTL_WAIT_LOOP_TIME_OUT;
}

void nand_cmd_reset(uint8_t chan, uint8_t bank)
{
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE;
    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce;
    g_channel_set_reg->reg_nand_cmd = 0xFF;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
}

void nand_cmd_hard_reset(uint8_t chan, uint8_t bank)
{
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE;
    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce;
    g_channel_set_reg->reg_nand_cmd = 0xFD;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
}

void nand_cmd_synchronous_reset(uint8_t chan, uint8_t bank)
{
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE;
    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce;
    g_channel_set_reg->reg_nand_cmd = 0xFC;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
}

void nand_cmd_reset_lun(uint8_t chan, uint8_t bank, uint8_t addr_list[], uint8_t list_size)
{
    uint16_t i;
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE;
    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce;
    g_channel_set_reg->reg_nand_cmd = 0xFA;
    for (i = 0; i < list_size; i++)
    {
        g_channel_set_reg->reg_nand_adr = addr_list[i];
    }
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
}

void nand_cmd_zq_calibration_long(uint8_t chan, uint8_t bank)
{
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE;

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce;
    g_channel_set_reg->reg_nand_cmd = 0xF9;
    g_channel_set_reg->reg_nand_adr = bank;                 // set NV-DDR3 config reg

    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
}

void nand_cmd_zq_calibration_short(uint8_t chan, uint8_t bank)
{
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE;

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce;
    g_channel_set_reg->reg_nand_cmd = 0xD9;
    g_channel_set_reg->reg_nand_adr = bank;                 // set NV-DDR3 config reg

    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
}

void nand_cmd_set_feature(uint8_t chan, uint8_t bank, uint8_t nand_adr, uint16_t wdt[CMD_FEATURE_NUM])
{
    uint16_t k;
    uint16_t i;
    RW_SET_BUSY busy_setting;
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE; // set cmd hold

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce; // set CEn
    g_channel_set_reg->reg_nand_cmd = 0xEF;                     // set feature cmd
    g_channel_set_reg->reg_nand_adr = nand_adr;                 // set NV-DDR3 config reg

    // wait tADL of set feature
    for (k = 0; k < 200; k++);

    // loop to set wdt data
    for (i = 0; i < CMD_FEATURE_NUM; i++)
    {
        g_channel_set_reg->reg_nand_wdt = wdt[i];
    }

    // check ready/busy
    busy_setting.ctrl.reg_frc_rds = STS_BUSY;
    busy_setting.ctrl.reg_busy_ps = STS_BUSY;
    busy_setting.ctrl.reg_tmr_sel = 0;
    g_channel_set_reg->reg_set_busy.whole = busy_setting.whole;
    // turn off CEn
    g_channel_set_reg->reg_ena_cen = 0xff;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
}

void nand_cmd_get_feature(uint8_t chan, uint8_t bank, uint8_t nand_adr, uint16_t rdt[CMD_FEATURE_NUM])
{
    uint16_t k;
    uint16_t i;
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE; // set cmd hold

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce; // set CEn
    g_channel_set_reg->reg_nand_cmd = 0xEE;                     // get feature cmd
    g_channel_set_reg->reg_nand_adr = nand_adr;                 // set NV-DDR3 config reg

    // wait time of get feature
    for (k = 0; k < UINT16_MAX; k++);

    // read data
    for (i = 0; i < CMD_FEATURE_NUM;)
    {
        g_channel_set_reg->reg_nand_rdt = 0x1;
        rdt[i++] = g_channel_set_reg->nand_rdata & 0xffff;
        rdt[i++] = g_channel_set_reg->nand_rdata >> 16;
    }
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
}

uint32_t nand_cmd_read_unique_id(uint8_t chan, uint8_t bank, uint8_t nand_adr, uint8_t rdr[CMD_READ_UID_NUM])
{
    uint16_t k;
    uint16_t i;
    uint32_t unique_id = 0;
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE; // set cmd hold

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce; // set CEn
    g_channel_set_reg->reg_nand_cmd = 0xED;                     // read unique id cmd
    g_channel_set_reg->reg_nand_adr = nand_adr;                 // set NV-DDR3 config reg

    // wait time of read id
    for (k = 0; k < UINT16_MAX; k++);

    // read data
    for (i = 0; i < CMD_READ_UID_NUM;)
    {
        g_channel_set_reg->reg_nand_rdt = 0x1;
        rdr[i++] = g_channel_set_reg->nand_rdata & 0xff;
        rdr[i++] = (g_channel_set_reg->nand_rdata >>  8) & 0xff;
        rdr[i++] = (g_channel_set_reg->nand_rdata >> 16) & 0xff;
        rdr[i++] = (g_channel_set_reg->nand_rdata >> 24) & 0xff;
    }

    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
    return unique_id;
}

void nand_cmd_odt_configure(uint8_t chan, uint8_t bank, uint8_t addr_list[], uint8_t addr_num, uint16_t wdt_list[], uint8_t wdt_num)
{
    uint16_t k;
    uint16_t i;
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE; // set cmd hold

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce; // set CEn
    g_channel_set_reg->reg_nand_cmd = 0xE2;                     // set odt configure cmd

    for (i = 0; i < addr_num; i++)
    {
        g_channel_set_reg->reg_nand_adr = addr_list[i];         // set NV-DDR3 config reg
    }

    // wait tADL
    for (k = 0; k < 150; k++);
    // loop to set wdt data

    for (i = 0; i < wdt_num; i++)
    {
        g_channel_set_reg->reg_nand_wdt = wdt_list[i];
    }
    // turn off CEn
    g_channel_set_reg->reg_ena_cen = 0xff;

    // wait ready
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
    while (g_channel_set_reg->status_rdy != 0xffff);
}

void nand_cmd_read_id(uint8_t chan, uint8_t bank, uint8_t nand_adr, uint8_t rdr[CMD_READ_ID_NUM])
{
    uint16_t k;
    uint16_t i;
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE; // set cmd hold

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce; // set CEn
    g_channel_set_reg->reg_nand_cmd = 0x90;                     // read id cmd
    g_channel_set_reg->reg_nand_adr = nand_adr;                 // set NV-DDR3 config reg

    // wait time of read id
    for (k = 0; k < UINT16_MAX; k++);

    // read data
    for (i = 0; i < CMD_READ_ID_NUM;)
    {
        g_channel_set_reg->reg_nand_rdt = 0x1;
#if defined(NAND_BICS5)
        rdr[i++] = g_channel_set_reg->nand_rdata & 0xff;
        rdr[i++] = (g_channel_set_reg->nand_rdata >> 8) & 0xff;
#elif defined(NAND_YX39070)
        rdr[i++] = g_channel_set_reg->nand_rdata & 0xff;
        rdr[i++] = (g_channel_set_reg->nand_rdata >> 16) & 0xff;
#endif
    }
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
}

void nand_cmd_read_parameter_page(uint8_t chan, uint8_t bank, uint8_t nand_adr, uint8_t rdr[CMD_READ_PARA_PAGE_NUM])
{
    uint16_t k;
    uint16_t i;
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE; // set cmd hold

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce; // set CEn
    g_channel_set_reg->reg_nand_cmd = 0xEC;                     // read parameter page cmd
    g_channel_set_reg->reg_nand_adr = nand_adr;                 // set NV-DDR3 config reg

    // wait time of read parameter page
    for (k = 0; k < UINT16_MAX; k++);

    // read data
    for (i = 0; i < CMD_READ_PARA_PAGE_NUM;)
    {
        g_channel_set_reg->reg_nand_rdt = 0x1;
        rdr[i++] = g_channel_set_reg->nand_rdata & 0xff;
        rdr[i++] = (g_channel_set_reg->nand_rdata >>  8) & 0xff;
        rdr[i++] = (g_channel_set_reg->nand_rdata >> 16) & 0xff;
        rdr[i++] = (g_channel_set_reg->nand_rdata >> 24) & 0xff;
    }
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
}

uint8_t nand_cmd_read_status_enhanced(uint8_t chan, uint8_t bank, uint8_t addr_list[], uint8_t list_size)
{
    uint16_t k;
    uint16_t i;
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE;

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce;
    g_channel_set_reg->reg_nand_cmd = 0x78;

    for (i = 0; i < list_size; i++)
    {
        g_channel_set_reg->reg_nand_adr = addr_list[i];
    }

    // wait time of read status
    for (k = 0; k < UINT16_MAX; k++);

    // read status data
    g_channel_set_reg->reg_nand_rdt = 0x1;
    g_channel_set_reg->reg_rds_dat = g_channel_set_reg->nand_rdata & 0xff;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;

    return g_channel_set_reg->reg_rds_dat;
}

uint8_t nand_cmd_read_status_lun_previous(uint8_t chan, uint8_t bank)
{
    uint16_t k;
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE;

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce;
    g_channel_set_reg->reg_nand_cmd = 0x73;

    // wait time of read status
    for (k = 0; k < UINT16_MAX; k++);

    // read status data
    g_channel_set_reg->reg_nand_rdt = 0x1;
    g_channel_set_reg->reg_rds_dat = g_channel_set_reg->nand_rdata & 0xff;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;

    return g_channel_set_reg->reg_rds_dat;
}

uint8_t nand_cmd_read_status_lun(uint8_t chan, uint8_t bank)
{
    uint16_t k;
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE;

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce;
    g_channel_set_reg->reg_nand_cmd = 0x71;

    // wait time of read status
    for (k = 0; k < UINT16_MAX; k++);

    // read status data
    g_channel_set_reg->reg_nand_rdt = 0x1;
    g_channel_set_reg->reg_rds_dat = g_channel_set_reg->nand_rdata & 0xff;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;

    return g_channel_set_reg->reg_rds_dat;
}

uint8_t nand_cmd_read_status(uint8_t chan, uint8_t bank)
{
    uint16_t k;
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE;

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce;
    g_channel_set_reg->reg_nand_cmd = 0x70;

    // wait time of read status
    for (k = 0; k < UINT16_MAX; k++);

    // read status data
    g_channel_set_reg->reg_nand_rdt = 0x1;
    g_channel_set_reg->reg_rds_dat = g_channel_set_reg->nand_rdata & 0xff;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;

    return g_channel_set_reg->reg_rds_dat;
}

void nand_cmd_set_feature_by_lun(uint8_t chan, uint8_t bank, uint8_t lun_adr, uint8_t nand_adr, uint16_t wdt[CMD_FEATURE_NUM])
{
    uint16_t k;
    uint16_t i;
    RW_SET_BUSY busy_setting;
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE; // set cmd hold

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce; // set CEn
    g_channel_set_reg->reg_nand_cmd = 0xD5;                     // set feature by lun cmd

    g_channel_set_reg->reg_nand_adr = lun_adr;                  // set NV-DDR3 config reg
    g_channel_set_reg->reg_nand_adr = nand_adr;                 // set NV-DDR3 config reg

    // wait tADL of set feature
    for (k = 0; k < 150; k++);

    // loop to set wdt data
    for (i = 0; i < CMD_FEATURE_NUM; i++)
    {
        g_channel_set_reg->reg_nand_wdt = wdt[i];
    }

    // check ready/busy
    busy_setting.ctrl.reg_frc_rds = STS_BUSY;
    busy_setting.ctrl.reg_busy_ps = STS_BUSY;
    busy_setting.ctrl.reg_tmr_sel = 0;
    g_channel_set_reg->reg_set_busy.whole = busy_setting.whole;
    // turn off CEn
    g_channel_set_reg->reg_ena_cen = 0xff;

    // wait "set feature" ready
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
    while (g_channel_set_reg->status_rdy != 0xffff);
}

void nand_cmd_get_feature_by_lun(uint8_t chan, uint8_t bank, uint8_t lun_adr, uint8_t nand_adr, uint16_t rdt[CMD_FEATURE_NUM])
{
    uint16_t k;
    uint16_t i;
    g_nand_set_reg->reg_index = chan;
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE; // set cmd hold

    g_channel_set_reg->reg_cmd_lunce = bank;
    g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce; // set CEn
    g_channel_set_reg->reg_nand_cmd = 0xD4;                     // get feature by lun cmd

    g_channel_set_reg->reg_nand_adr = lun_adr;                  // set NV-DDR3 config reg
    g_channel_set_reg->reg_nand_adr = nand_adr;                 // set NV-DDR3 config reg

    // wait time of get feature
    for (k = 0; k < UINT16_MAX; k++);

    // read data
    for (i = 0; i < CMD_FEATURE_NUM;)
    {
        g_channel_set_reg->reg_nand_rdt = 0x1;
        rdt[i++] = g_channel_set_reg->nand_rdata & 0xffff;
        rdt[i++] = g_channel_set_reg->nand_rdata >> 16;
    }
    g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
}

uint32_t nand_factory_defective_check(uint16_t blk, uint8_t bank, uint8_t chan, uint8_t planeX)
{
    uint16_t t_buff_lsb = hal_bmu_alloc_buff(SRAM_BUF, CPUA_GET);
    uint16_t t_buff_msb = hal_bmu_alloc_buff(SRAM_BUF, CPUA_GET);
    uint32_t t_buff_mem_low = hal_bmu_get_sram_mem_addr(t_buff_lsb);
    uint32_t t_buff_mem_msb = hal_bmu_get_sram_mem_addr(t_buff_msb);
    uint8_t frag = (uint8_t)(planeX << PLANE_SHIFT_FRAG);
    uint32_t lsb_p4k = get_p4k(blk, 0              , bank, chan, frag);
//    uint32_t msb_p4k = get_p4k(blk, (PAGE_QNTY - 1), bank, chan, frag);

    hal_nctl_erase_single_blk(ERS_SINGE_CPTR, bank, chan, blk, planeX);
    nand_erase_cmd_wait_and_check(chan, bank);

    hal_nctl_read_4k_to_buff(READ_CPTR, t_buff_lsb, lsb_p4k);
    //hal_nctl_read_4k_to_buff(READ_CPTR, t_buff_msb, msb_p4k);
    if(hal_bmu_wait_read_buff_vld() == RET_ERR)
    {
        log_print(info, "read 4k to buff fail\r\n");
        hal_bmu_clear_errq();
    }
    hal_bmu_clear_smrdq();
    //hal_bmu_wait_read_buff_vld();

    // check FFh first byte of(user data or spare data) of the LSB page in the first WL
    uint8_t lsb_byte = (*(volatile uint32_t *)t_buff_mem_low) & 0xFF;
    // check FFh first byte of(user data or spare data) of the MSB page in the last WL
    uint8_t msb_byte = (*(volatile uint32_t *)t_buff_mem_msb) & 0xFF;
    hal_bmu_cpu_free_buff(t_buff_lsb);
    hal_bmu_cpu_free_buff(t_buff_msb);
    if ((lsb_byte == 0x00))// || (msb_byte == 0x00))
    {
        return NCTL_CHECK_STATUS_BAD_BLOCK;
    }

    return RET_OK;
}

uint32_t nand_data_training(uint8_t chan, uint8_t bank)
{
    {
        g_nand_set_reg->reg_index = chan;
        g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = ENABLE; // set cmd hold
        g_channel_set_reg->reg_cmd_lunce = bank;
        g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce; // set CEn
        g_channel_set_reg->reg_nand_cmd = 0x1B;                     // ODT Disable(1Bh)
        g_channel_set_reg->reg_ena_cen = 0xff;
        g_channel_set_reg->reg_func_except.ctrl.hold_mic_pro = DISABLE;
    }
    // high sppeed setting(02h)
    uint16_t high_sppeed_setting[CMD_FEATURE_NUM] = {0x4747, 0x4444, 0x0, 0x0};
    nand_cmd_set_feature(chan, bank, 0x02, high_sppeed_setting);

    // Driver strength setting (10h)
    uint16_t driver[CMD_FEATURE_NUM] = {0x0404, 0x0, 0x0, 0x0};
    nand_cmd_set_feature(chan, bank, 0x10, driver);

    // flash memory side ZQ calibration
    // controller side ZQ calibration
    nand_cmd_zq_calibration_short(chan, bank);
    nand_cmd_zq_calibration_long(chan, bank);
    for(uint16_t i = 0; i < UINT16_MAX; i++);

    // dcc enable (20h)
    uint16_t dcc_en[CMD_FEATURE_NUM] = {0x0101, 0x0, 0x0, 0x0};
    nand_cmd_set_feature(chan, bank, 0x20, dcc_en);

    uint16_t t_buff = hal_bmu_alloc_buff(SRAM_BUF, CPUA_GET);
    uint32_t t_buff_mem = hal_bmu_get_sram_mem_addr(t_buff);
    uint32_t t_p4k = get_p4k(0, 0, bank, chan, 0);
    g_cmd_info_reg->reg_proc_info = bank;

    for (uint16_t i = 0; i < (4096 + 16); i += 4) {
        (*(volatile uint32_t *)(t_buff_mem + i)) = 0x11111111;
    }
    hal_nctl_read_4k_to_buff(DCC_TRAINING_CPTR, t_buff, t_p4k);
    hal_bmu_wait_read_buff_vld();

    for (uint16_t i = 0; i < (4096 + 16); i += 4) {
        (*(volatile uint32_t *)(t_buff_mem + i)) = 0x11111111;
    }
    hal_nctl_read_4k_to_buff(READ_DQ_TRAINING_CPTR, t_buff, t_p4k);
    hal_bmu_wait_read_buff_vld();

    // {
    //     g_nand_set_reg->reg_index = chan;
    //     g_channel_set_reg->rw_func_except.ctrl.hold_mic_pro = ENABLE; // set cmd hold
    //     g_channel_set_reg->reg_cmd_lunce = bank;
    //     g_channel_set_reg->reg_ena_cen = g_channel_set_reg->phy_ce; // set CEn
    //     g_channel_set_reg->reg_nand_cmd = 0x1C;                     // ODT enable(1Ch)
    //     g_channel_set_reg->reg_ena_cen = 0xff;
    //     g_channel_set_reg->rw_func_except.ctrl.hold_mic_pro = DISABLE;
    // }

    // for (uint16_t i = 0; i < (4096 + 16); i += 4) {
    //     (*(volatile uint32_t *)(t_buff_mem + i)) = 0x22222222;
    // }
    // nctl_program_4k(WRITE_DQ_TRAINING_CPTR, t_buff, t_p4k);
    // nand_write_cmd_wait_and_check(chan, bank);

    // for (uint16_t i = 0; i < (4096 + 16); i += 4) {
    //     (*(volatile uint32_t *)(t_buff_mem + i)) = 0x11111111;
    // }
    // nctl_read_4k(WRITE_DQ_TRAINING_READ_CPTR, t_buff, t_p4k);
    // nctl_read_vld();

    // // dcc disable (20h)
    // uint16_t dcc_disable[CMD_FEATURE_NUM] = {0x0, 0x0, 0x0, 0x0};
    // nand_cmd_set_feature(chan, bank, 0x20, dcc_disable);

    // for (uint16_t i = 0; i < (4096 + 16); i += 4) {
    //     (*(volatile uint32_t *)(t_buff_mem + i)) = 0x11111111;
    // }
    // nctl_read_4k(READ_CPTR, t_buff, t_p4k);
    // nctl_read_vld();

    return RET_OK;
}
