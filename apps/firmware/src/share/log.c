
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "log.h"

/*
void normal_log_multi_blk_block_update(update_blk_type type);
void important_log_multi_blk_block_update(update_blk_type type);

update_blk_func log_update_blk_functions[LOG_SAVE_TYPE_MAX] =
{
    normal_log_multi_blk_block_update,
    important_log_multi_blk_block_update
};
sys_multi_blk_pa log_multi_blk[LOG_SAVE_TYPE_MAX*LOG_NEED_MULTI_BLK_NUM];
root_nand_pa_mgr_t g_log_nand_pa_mgr[LOG_SAVE_TYPE_MAX];

root_nandspace_t g_log_origin_nandspace =
{
    .p_multi_blks = log_multi_blk,
    .p_nand_pa_mgr = g_log_nand_pa_mgr,
    .p_update_blk_functions = log_update_blk_functions,
    .max_save_type_num = LOG_SAVE_TYPE_MAX,
    };

void log_dram_buf_init(void);
void log_init_nand_pa_mg(void);
int32_t nand_erase_cmd_wait_and_check(uint8_t chan, uint8_t bank);
void erase_log_blks(void)
{
    sys_multi_blk_pa *p_cur_multi_blks;
    for(uint8_t i = 0; i < LOG_SAVE_TYPE_MAX*LOG_NEED_MULTI_BLK_NUM; i++)
    {
        p_cur_multi_blks =  log_multi_blk + i;
        nctl_erase_blk(ERS_CPTR, p_cur_multi_blks->bank, p_cur_multi_blks->ch_id, p_cur_multi_blks->blk_id, 0);
        for(uint16_t j = 0; j < UINT16_MAX; j++);
        if (nand_erase_cmd_wait_and_check(p_cur_multi_blks->ch_id, p_cur_multi_blks->bank)!= OK){
            continue;
        }
    }
}

//get nand space when opencard
int32_t log_get_nand_multi_blk(void)
{
    return space_get_sys_item_multi_blk(NAND_SYS_DATA_LOG, (g_log_origin_nandspace.p_multi_blks), LOG_NEED_MULTI_BLK_NUM, &(g_log_origin_nandspace.total_multi_blk_num));
}

void log_be_persist_init(void)
{
    erase_log_blks();
    init_root_nand_pa_mgr(g_log_origin_nandspace.p_nand_pa_mgr, g_log_origin_nandspace.p_multi_blks, g_log_origin_nandspace.total_multi_blk_num, g_log_origin_nandspace.max_save_type_num);
}

void log_multi_blk_block_update(root_nand_pa_mgr_t *p_log_nand_pa_mgr, uint32_t* log_len)
{
    root_multi_blk_block_update_and_erase_next_blk(p_log_nand_pa_mgr);
    if(p_log_nand_pa_mgr->origin_multi_blk_index >= (p_log_nand_pa_mgr->origin_multi_blk_num - 1)){
        (*log_len) -= OPEN_LOG_CNT_PER_BLK;
    }
}

void normal_log_multi_blk_block_update(update_blk_type type)
{
    log_multi_blk_block_update(g_log_nand_pa_mgr + LOG_SAVE_TYPE_NORMAL, g_log_mgr.write_log_len + LOG_SAVE_TYPE_NORMAL);
}

void important_log_multi_blk_block_update(update_blk_type type)
{
    log_multi_blk_block_update(g_log_nand_pa_mgr + LOG_SAVE_TYPE_IMPORTANT, g_log_mgr.write_log_len + LOG_SAVE_TYPE_IMPORTANT);
}

void write_log_to_nand(uint8_t type, root_dram_buf_mgr_t *p_dram_buf_mgr)
{
    uint16_t program_times =  ROUND_UP(p_dram_buf_mgr->dram_index, P4K_NUM_IN_PROU_SLC);
    write_root_dramdata_to_nand(&(g_log_origin_nandspace.p_nand_pa_mgr[type].origin_data_pa), p_dram_buf_mgr->dram_buf, program_times,
                                g_log_origin_nandspace.p_update_blk_functions[type], UPDATE_BLK_TYPE_BUTT);
    p_dram_buf_mgr->dram_index = 0;
}

int8_t receive_write_log_msg_or_abnormal_handle(int8_t *msg_buffer, uint8_t len)
{
    write_log_to_nand_msg_t *p_log_to_nand_msg =  (write_log_to_nand_msg_t *)msg_buffer;
    write_log_to_nand(p_log_to_nand_msg->type, &(p_log_to_nand_msg->dram_buf_mgr));
    return SUCCESS;
}
*/
