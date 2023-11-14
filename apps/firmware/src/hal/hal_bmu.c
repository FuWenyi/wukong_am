#include <stdint.h>
#include "hal_bmu.h"
#include "hal_bmu_register.h"
#include "hal_err_code.h"
#include "../comm/help_op.h"
#include "../inc/register.h"


typedef struct tag_bmu_get_buff_num
{
    uint32_t buff_type;
    uint16_t (*get_free_buff_num)(void);
    uint16_t (*get_used_buff_num)(uint32_t);
    uint16_t (*get_buff_threshold)(uint32_t);
    void (*set_buff_threshold)(uint32_t, uint16_t);
} bmu_get_buff_num;

bmu_get_buff_num g_buff_num_table[] =
{
    {DRAM_BUF, hal_bmu_get_dram_free_buff_num, hal_bmu_get_dram_used_buff_num, hal_bmu_get_dram_buff_threshold, hal_bmu_set_dram_buff_threshold},
    {SRAM_BUF, hal_bmu_get_sram_free_buff_num, hal_bmu_get_sram_used_buff_num, hal_bmu_get_sram_buff_threshold, hal_bmu_set_sram_buff_threshold},
    {PRTY_BUF, hal_bmu_get_parity_free_buff_num, hal_bmu_get_parity_used_buff_num, hal_bmu_get_parity_buff_threshold, hal_bmu_set_parity_buff_threshold},
};

uint16_t hal_bmu_alloc_buff(uint32_t buff_type, uint32_t master_type)
{
    rb_cafp_set = CAFP_ACT | buff_type | master_type;
    while(rb_cafp_set & CAFP_ACT)
    {
        // need consider timeout mechanism in solution version
    }

    if (rb_cafp_rlt == 0)
    {
        return UINT16_MAX; // TODO:check reason here?
    }

    return rw_cafp_ptr;
}

void hal_bmu_cpu_free_buff(uint16_t buff_ptr)
{
    rd_carl_set = CARL_ACT | (buff_ptr << 16);
    return;
}

uint32_t hal_bmu_get_sram_hw_addr(uint16_t buff_ptr) // TODO
{
    return (buff_ptr & 0x1ff) << 12;
}

uint32_t hal_bmu_get_sram_mem_addr(uint16_t buff_ptr)
{
    return ((uint32_t)(buff_ptr & 0x1ff) << 12) + SRAM_MEM_BASE;
}

uint32_t hal_bmu_get_sram_meta_addr(uint16_t buff_ptr)
{
    return ((uint32_t)(buff_ptr & 0x1ff) << 12) + SRAM_MEM_BASE + 0x200000;
}

uint16_t hal_bmu_get_sram_bfp_from_cpu_addr(uint32_t addr)
{
    return (uint16_t)((addr - SRAM_MEM_BASE) >> 12) + (uint16_t)SRAM_BUFF_POINTER_BASE;
}

uint16_t hal_bmu_get_sram_bfp_from_hw_addr(uint32_t addr) //TODO
{
    return (uint16_t) (addr >> 12) | (uint16_t)SRAM_BUFF_POINTER_BASE;
}

uint32_t hal_bmu_get_dram_mem_addr(uint16_t buff_ptr)
{
    return ((buff_ptr << 12) + DRAM_DATA_BASE);
}

uint32_t hal_bmu_get_dram_meta_addr(uint16_t buff_ptr)
{
    return ((buff_ptr << 8) + DRAM_META_BASE);
}

uint32_t hal_bmu_is_support_get_used_buff_num(uint32_t buff_type, uint32_t master_type)
{
    // no register for nand used dram buff
    if ((buff_type == DRAM_BUF) && (master_type == NAND_GET))
    {
        return 0;
    }

    // no register for nand used sram buff
    if ((buff_type == SRAM_BUF) && (master_type == NAND_GET))
    {
        return 0;
    }

    return 1;
}

uint16_t hal_bmu_get_dram_free_buff_num(void)
{
    return rw_dlnk_num;
}

uint16_t hal_bmu_get_sram_free_buff_num(void)
{
    return rw_slnk_num;
}

uint16_t hal_bmu_get_parity_free_buff_num(void)
{
    return rw_plnk_num;
}

// cannot coding as hash table due to armcc err:expression must have a constant value
uint16_t hal_bmu_get_dram_used_buff_num(uint32_t master_type)
{
    if (master_type == HOST_GET)
    {
        return rw_htdb_num + rw_hrdb_num;
    }
    else if (master_type == DWDM_GET)
    {
        return rw_gcdb_num; // need to check whether the relation is correct?
    }
    else if (master_type == CPUA_GET)
    {
        return rw_cadb_num;
    } else
    {
        return UINT16_MAX;
    }
}

uint16_t hal_bmu_get_sram_used_buff_num(uint32_t master_type)
{
    if (master_type == HOST_GET)
    {
        return rw_htsb_num + rw_hrsb_num;
    }
    else if (master_type == DWDM_GET)
    {
        return rw_gcsb_num + rw_tbsb_num + rw_rasb_num;
    }
    else if (master_type == CPUA_GET)
    {
        return rw_casb_num;
    }
    else
    {
        return UINT16_MAX;
    }
}

uint16_t hal_bmu_get_parity_used_buff_num(uint32_t master_type)
{
    if (master_type == HOST_GET)
    {
        return rw_htpb_num + rw_hrpb_num;
    }
    else if (master_type == NAND_GET)
    {
        return rw_ndpb_num;
    }
    else if (master_type == DWDM_GET)
    {
        return rw_gcpb_num + rw_gcpb_num + rw_rapb_num;
    }
    else if (master_type == CPUA_GET)
    {
        return rw_capb_num;
    }
    else
    {
        return UINT16_MAX;
    }
}

uint16_t hal_bmu_get_dram_buff_threshold(uint32_t master_type)
{
    if (master_type == HOST_GET)
    {
        return rw_htdb_thr;
    }
    else if (master_type == DWDM_GET)
    {
        return rw_gcdb_thr;
    }
    else if (master_type == CPUA_GET)
    {
        return rw_cadb_thr;
    } else
    {
        return UINT16_MAX;
    }
}

uint16_t hal_bmu_get_sram_buff_threshold(uint32_t master_type)
{
    if (master_type == HOST_GET)
    {
        return rw_htsb_thr;
    }
    else if (master_type == DWDM_GET)
    {
        return rw_gcsb_thr;
    }
    else if (master_type == CPUA_GET)
    {
        return rw_casb_thr;
    }
    else
    {
        return UINT16_MAX;
    }
}

uint16_t hal_bmu_get_parity_buff_threshold(uint32_t master_type)
{
    if (master_type == HOST_GET)
    {
        return rw_htpb_thr;
    }
    else if (master_type == NAND_GET)
    {
        return rw_ndpb_thr;
    }
    else if (master_type == DWDM_GET)
    {
        return rw_gcpb_thr;
    }
    else if (master_type == CPUA_GET)
    {
        return rw_capb_thr;
    }
    else
    {
        return UINT16_MAX;
    }
}

void hal_bmu_set_dram_buff_threshold(uint32_t master_type, uint16_t threshold)
{
    if (master_type == HOST_GET)
    {
        rw_htdb_thr = threshold;
    }
    else if (master_type == DWDM_GET)
    {
        rw_gcdb_thr = threshold;
    }
    else if (master_type == CPUA_GET)
    {
        rw_cadb_thr = threshold;
    }
    else
    {
        //assert
    }

    return;
}

void hal_bmu_set_sram_buff_threshold(uint32_t master_type, uint16_t threshold)
{
    if (master_type == HOST_GET)
    {
        rw_htsb_thr = threshold;
    }
    else if (master_type == DWDM_GET)
    {
        rw_gcsb_thr = threshold;
    }
    else if (master_type == CPUA_GET)
    {
        rw_casb_thr = threshold;
    }
    else
    {
        //assert
    }

    return;
}

void hal_bmu_set_parity_buff_threshold(uint32_t master_type, uint16_t threshold)
{
    if (master_type == HOST_GET)
    {
        rw_htpb_thr = threshold;
    }
    else if (master_type == NAND_GET)
    {
        rw_ndpb_thr = threshold;
    }
    else if (master_type == DWDM_GET)
    {
        rw_gcpb_thr = threshold;
    }
    else if (master_type == CPUA_GET)
    {
        rw_capb_thr = threshold;
    }
    else
    {
        //assert;
    }

    return;
}

uint16_t hal_bmu_get_free_buff_num_by_type(uint32_t buff_type)
{
    uint8_t i = 0;
    uint8_t max = sizeof(g_buff_num_table) / sizeof(g_buff_num_table[0]);

    while (i < max)
    {
        if (buff_type == g_buff_num_table[i].buff_type)
        {
            return g_buff_num_table[i].get_free_buff_num();
        }
        i++;
    };

    return UINT16_MAX;
}

uint16_t hal_bmu_get_used_buff_num_by_type(uint32_t buff_type, uint32_t master_type)
{
    uint8_t i = 0;
    uint8_t max = sizeof(g_buff_num_table) / sizeof(g_buff_num_table[0]);

    while (i < max)
    {
        if (buff_type == g_buff_num_table[i].buff_type)
        {
            return g_buff_num_table[i].get_used_buff_num(master_type);
        }
        i++;
    };

    return UINT16_MAX;
}

uint16_t hal_bmu_get_buff_threshold_by_type(uint32_t buff_type, uint32_t master_type)
{
    uint8_t i = 0;
    uint8_t max = sizeof(g_buff_num_table) / sizeof(g_buff_num_table[0]);

    while (i < max)
    {
        if (buff_type == g_buff_num_table[i].buff_type)
        {
            return g_buff_num_table[i].get_buff_threshold(master_type);
        }
        i++;
    };

    return UINT16_MAX;
}

void hal_bmu_set_buff_threshold(uint32_t buff_type, uint32_t master_type, uint16_t threshold)
{
    uint8_t i = 0;
    uint8_t max = sizeof(g_buff_num_table) / sizeof(g_buff_num_table[0]);

    while (i < max)
    {
        if (buff_type == g_buff_num_table[i].buff_type)
        {
            g_buff_num_table[i].set_buff_threshold(master_type, threshold);
            return;
        }
        i++;
    };

    return;
}

void hal_bmu_wait_ddma_done(void)
{
    while(rb_ddma_act);
}

uint32_t hal_bmu_config_hw_ddr_to_cpu_ddr(uint64_t hw_dram_addr)
{
    rb_cdir_bas = (uint8_t) (hw_dram_addr >> 31);
    return ((((uint32_t) hw_dram_addr) & 0x7fffffff) + DRAM_MEM_BASE);
}

uint32_t hal_bmu_get_cpu_ddr_to_hw_ddr(uint32_t cpu_dram_addr)
{
    uint8_t bias = rb_cdir_bas;
    return (cpu_dram_addr - DRAM_MEM_BASE) + (bias << 31);
}

uint32_t hal_bmu_start_ddma_sram_to_dram(uint32_t sram_hw_addr, uint64_t dram_hw_addr, uint16_t dma_len)
{
    if (dram_hw_addr & 0xf || sram_hw_addr & 0xf)
    {
        //log_print(fatal, "sram_hw_addr: 0x%x, dram_hw_addr: 0x%x should 16byte aligned", sram_hw_addr, dram_hw_addr);
        return RET_ERR;
    }
    if (dma_len & 0x7)
    {
        //log_print(fatal, "dma_len: 0x%x should by 8byte aligned.", dma_len);
        return RET_ERR;
    }

    while(rb_ddma_act);
    rd_ddma_dad = (uint32_t) dram_hw_addr >> 4;
    rd_ddma_sad = sram_hw_addr >> 4;
    rw_ddma_len = dma_len >> 3;
    rb_ddma_wen = DDMA_SRAM_TO_DRAM;
    rb_ddma_act = 0x1;

    return RET_OK;
}

uint32_t hal_bmu_start_ddma_dram_to_sram(uint64_t dram_hw_addr, uint32_t sram_hw_addr, uint16_t dma_len)
{
    if (dram_hw_addr & 0xf || sram_hw_addr & 0xf)
    {
        //log_print(fatal, "sram_hw_addr: 0x%x, dram_hw_addr: 0x%x should 16byte aligned", sram_hw_addr, dram_hw_addr);
        return RET_ERR;
    }
    if (dma_len & 0x7)
    {
        //log_print(fatal, "dma_len: 0x%x should by 8byte aligned.", dma_len);
        return RET_ERR;
    }

    while(rb_ddma_act);
    rd_ddma_dad = (uint32_t) dram_hw_addr >> 4;
    rd_ddma_sad = sram_hw_addr >> 4;
    rw_ddma_len = dma_len >> 3;
    rb_ddma_wen = DDMA_DRAM_TO_SRAM;
    rb_ddma_act = 1;

    return RET_OK;
}

uint32_t hal_bmu_wait_read_buff_vld(void)
{
    do {
        if (rb_errd_vld != 0)
        {
            return RET_ERR;//NCTL_READ_4K_CMD_ERR;
        }
        if (rb_smrd_vld == 0)
        {
            continue;
        }
        return RET_OK;
    } while (1);

    // log_print(debug, "[cmd:%d, p4k:%#x]read time out", g_cmd_info_reg->rw_cmd_type.whole, g_cmd_info_reg->reg_cmd_padr);
    // return NCTL_WAIT_LOOP_TIME_OUT;
}

uint32_t hal_bmu_wait_read_dram_vld(void)
{
    do {
        if (rb_errd_vld != 0)
        {
            return RET_ERR;//NCTL_READ_4K_CMD_ERR;
        }
        if (rb_trrd_vld == 0)
        {
            continue;
        }
        return RET_OK;
    } while (1);

    // log_print(debug, "[cmd:%d, p4k:%#x]read time out", g_cmd_info_reg->rw_cmd_type.whole, g_cmd_info_reg->reg_cmd_padr);
    // return NCTL_WAIT_LOOP_TIME_OUT;
}

void hal_bmu_clear_errq(void)
{
    rb_errd_vld = 1;
}

void hal_bmu_clear_smrdq(void)
{
    rb_smrd_vld = SMRD_CLR;
}

void hal_bmu_clear_trrdq(void)
{
    rb_trrd_vld = TRRD_CLR;
}

uint8_t hal_bmu_get_errq_read_ctx(HAL_BMU_READ_ERRQ_CTX *err_rd_ptr)
{
    if(rb_errd_vld)
    {
        err_rd_ptr->queue = rb_errd_que;
        err_rd_ptr->org_bfp = rw_errd_bfp;
        err_rd_ptr->bmu_ptr = rw_errd_ptr;
        err_rd_ptr->cmd_tag[0] = rd_errd_cid(0);
        err_rd_ptr->padr = rd_errd_pad;

        return TRUE;
    }

    return FALSE;
}

uint8_t hal_bmu_get_smrdq_read_ctx(HAL_BMU_READ_SMRDQ_CTX *smrd_ptr)
{
    if(rb_smrd_vld)
    {
        smrd_ptr->bmu_ptr = rw_smrd_bfp;
        //smrd_ptr->padr = rd_smrd_pad;
        return TRUE;
    }

    return FALSE;
}

uint8_t hal_bmu_get_trrdq_read_ctx(HAL_BMU_READ_TRRDQ_CTX *trrd_ptr)
{
    if(rb_trrd_vld)
    {
        trrd_ptr->bfp = rw_trrd_bfp;
        trrd_ptr->dmp = rw_trrd_dmp;
        //meta data
        return TRUE;
    }

    return FALSE;
}

uint32_t hal_bmu_get_program_bits(uint8_t start_flag, uint8_t length)
{
    return (uint32_t) (((uint64_t)1 << length) - 1) << (start_flag & 0x1f);
}

void hal_bmu_wait_program_done(uint8_t start_flag, uint8_t length)
{
    uint8_t reg_set = start_flag >> 5; // start_flag/32
    uint32_t mask = hal_bmu_get_program_bits(start_flag, length);
    while((rd_nddn_bit(reg_set) & mask) != mask);
}

void hal_bmu_clear_program_flag(uint8_t start_flag, uint8_t length)
{
    uint8_t reg_set = start_flag >> 5; // start_flag/32
    rd_nddn_bit(reg_set) = hal_bmu_get_program_bits(start_flag, length);
}

void hal_bmu_wait_multi_program_flag_reg(uint8_t start_reg_idx, uint8_t bit_cnt)
{
    uint8_t flag_reg_cnt = ROUND_UP(bit_cnt, 32);
    uint8_t reminder_bit = bit_cnt % 32;
    uint8_t i;

    for (i = start_reg_idx; i < (start_reg_idx + flag_reg_cnt); i++)
    {
        if ((reminder_bit != 0) && (i == (start_reg_idx + flag_reg_cnt - 1)))
        {
            hal_bmu_wait_program_done(i << 5, reminder_bit);
            break;
        }
        hal_bmu_wait_program_done(i << 5, 1 << 5);
    }

    return;
}

void hal_bmu_clear_multi_program_flag_reg(uint8_t start_reg_idx, uint8_t bit_cnt)
{
    uint8_t flag_reg_cnt = ROUND_UP(bit_cnt, 32);
    uint8_t reminder_bit = bit_cnt % 32;
    uint8_t i;

    for (i = start_reg_idx; i < (start_reg_idx + flag_reg_cnt); i++)
    {
        if ((reminder_bit != 0) && (i == (start_reg_idx + flag_reg_cnt - 1)))
        {
            hal_bmu_clear_program_flag(i << 5, reminder_bit);
            break;
        }
        hal_bmu_clear_program_flag(i << 5, 1 << 5);
    }

    return;
}
