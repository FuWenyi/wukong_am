#include "hal_nctl.h"
#include "hal_nctl_register.h"
#include "hal_err_code.h"
#include "../inc/global.h"
#include "../inc/register.h"
//#include "../share/log.h"
#include "../share/p4k.h"
//#include "../share/error_code.h"


void hal_nctl_cmd_send_base(NCTL_CMD_BASE cmd_base)
{
    rb_cmd_qidx = cmd_base.bmsg_que;

    //rw_cmd_othr = (cmd_base.bmsg_ltp << 8) | ((word)cmd_base.bmsg_lid);
    rw_cmd_dmpt = cmd_base.bmsg_dmp;
    rw_cmd_bptr = cmd_base.bmsg_bfp;
    rw_bmsg_fci = cmd_base.en_force_ecc | (cmd_base.ecc_port_num << 1);
    rd_cmd_padr = cmd_base.cmd_padr;
    rw_cmd_type = (RW_CMD_TYPE){
        .ctrl.reg_cmd_ptr = cmd_base.cmd_ptr,
        .ctrl.reg_cmd_pri = cmd_base.cmd_pri,
        .ctrl.reg_cmd_end = cmd_base.cmd_end,
        .ctrl.reg_cmd_dmap = cmd_base.cmd_dmap,
    }.whole;
}

void hal_nctl_read_sts(uint8_t cmd_ptr, uint8_t bank, uint8_t chan)
{
    hal_nctl_cmd_send_base((NCTL_CMD_BASE){
        .cmd_padr = get_p4k(0, 0, bank, chan, 0),
        .cmd_ptr = cmd_ptr,
        .cmd_end = TRUE,
    });
}

void hal_nctl_erase_multi_blk(uint8_t cmd_ptr, uint8_t bank, uint8_t chan, uint16_t blk)
{
    uint8_t ce_ch;
    ce_ch = (bank << CHAN_SHIFT) | chan;
    rd_cmd_padr = (blk << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) | (ce_ch << FRAG_SHIFT);
    rw_cmd_type = cmd_ptr | LOW_PRI | CMD_END;
}

void hal_nctl_erase_single_blk(uint8_t cmd_ptr, uint8_t bank, uint8_t chan, uint16_t blk, uint8_t planeX)
{
    uint8_t ce_ch;
    ce_ch = (bank << CHAN_SHIFT) | chan;
    rd_cmd_padr = (blk << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) | (ce_ch << FRAG_SHIFT) | (planeX << 2);
    rw_cmd_type = cmd_ptr | LOW_PRI | CMD_END;
}

void hal_nctl_program_buff_4k(uint8_t cmd_ptr, uint16_t buffer_ptr, uint32_t p4k, uint8_t end)
{
    rb_cmd_qidx = QLNK_SMQ;
    rd_cmd_padr = p4k;
    rw_cmd_bptr = buffer_ptr;
    rw_cmd_type = (RW_CMD_TYPE){
        .ctrl.reg_cmd_ptr = cmd_ptr,
        .ctrl.reg_cmd_pri = 0,
        .ctrl.reg_cmd_end = end,
        .ctrl.reg_cmd_dmap = 0,
    }.whole;
}

void hal_nctl_program_dram_4k(uint8_t cmd_ptr, uint64_t dram_addr, uint8_t dma_ptr, uint32_t p4k, uint8_t end)
{
    uint32_t dram_buf_id = (uint32_t)(dram_addr >> 12);
    rb_cmd_qidx = QLNK_TRQ;
    rd_cmd_padr = p4k;
    rw_cmd_dmpt = (word)(dram_buf_id >> 12);
    rw_cmd_bptr = (word)(dram_buf_id & 0xfff);
    rw_cmd_othr = (DBUF_KEEP << 8) | ((word)dma_ptr);
    rw_cmd_type = (RW_CMD_TYPE){
        .ctrl.reg_cmd_ptr = cmd_ptr,
        .ctrl.reg_cmd_pri = 0,
        .ctrl.reg_cmd_end = end,
        .ctrl.reg_cmd_dmap = 0,
    }.whole;
}

void hal_nctl_read_4k_to_buff(uint8_t cmd_ptr, uint16_t buffer_ptr, uint32_t p4k)
{
    rb_cmd_qidx = QLNK_SMQ;
    rw_cmd_bptr = buffer_ptr;
    rd_cmd_padr = p4k;
    rw_cmd_type = (RW_CMD_TYPE){
        .ctrl.reg_cmd_ptr = cmd_ptr,
        .ctrl.reg_cmd_pri = 0,
        .ctrl.reg_cmd_end = 0,
        .ctrl.reg_cmd_dmap = 0,
    }.whole;
}

void hal_nctl_read_4k_to_dram(uint8_t cmd_ptr, uint32_t dram_addr, uint32_t p4k)
{
    uint32_t dram_buf_id = dram_addr >> 12;
    rb_cmd_qidx = QLNK_TRQ;
    rd_cmd_padr = p4k;
    rw_cmd_dmpt = (word)(dram_buf_id >> 12);
    rw_cmd_bptr = (word)(dram_buf_id & 0xfff);
    rw_cmd_type = (RW_CMD_TYPE){
        .ctrl.reg_cmd_ptr = cmd_ptr,
        .ctrl.reg_cmd_pri = 0,
        .ctrl.reg_cmd_end = 0,
        .ctrl.reg_cmd_dmap = 0,
    }.whole;
}
