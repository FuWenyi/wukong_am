#ifndef _BMU_REGISTER_H_
#define _BMU_REGISTER_H_


/*******************************************************************************************************/
/********************                BMU rwister definition                  **************************/
/*******************************************************************************************************/

#define SRAM_MEM_SIZE          0x200000
#define SRAM_BUFF_SIZE         0x1000
#define SRAM_BUFF_POINTER_BASE 0xA00  // 2560
//#define SMDT_BASE   0x5 // 0x5 << 9 == SRAM_BUFF_POINTER_BASE

#define rd_trrd_tmt      (*(volatile dwrd *)(BMU_REG_BASE+0x88)) // meta of nand read data

/* TODO: wait update define -- start */
#define BUFFER_THR_REG 0x100
#define rw_htdl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x00))
#define rw_htdb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x02))
// #define rw_htsl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x04))
// #define rw_htsb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x06))
#define rw_htpl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x08))
#define rw_htpb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x0a))
#define rw_ndpl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x0c))
//#define rw_ndpb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x0e))

#define rw_hrdl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x10))
#define rw_hrdb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x12))
#define rw_hrsl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x14))
#define rw_hrsb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x16))
#define rw_hrpl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x18))
#define rw_hrpb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x1a))

// #define rw_gcdl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x1c))
// #define rw_gcdb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x1e))
// #define rw_gcsl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x20))
// #define rw_gcsb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x22))
#define rw_gcpl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x24))
#define rw_gcpb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x26))

#define rw_tbsl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x28))
//#define rw_tbsb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x2a))
#define rw_tbpl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x2c))
//#define rw_tbpb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x2e))

#define rw_rasl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x30))
#define rw_rasb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x32))
#define rw_rapl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x34))
#define rw_rapb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x36))

#define rw_cadl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x38))
#define rw_cadb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x3a))
#define rw_casl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x3c))
#define rw_casb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x3e))
#define rw_capl_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x40))
#define rw_capb_thr     (*(volatile word *)(BMU_REG_BASE + BUFFER_THR_REG + 0x42))

#define BUFFER_NUM_REG 0x180
#define rw_dlnk_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x00))
#define rw_htdb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x02))
#define rw_hrdb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x04))
#define rw_gcdb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x06))
#define rw_cadb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x08))

#define rw_slnk_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x0a))
#define rw_htsb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x0c))
#define rw_hrsb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x0e))
#define rw_gcsb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x10))
#define rw_tbsb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x12))
#define rw_rasb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x14))
#define rw_casb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x16))

#define rw_plnk_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x18))
#define rw_ndpb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x1a))
#define rw_htpb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x1c))
#define rw_hrpb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x1e))
#define rw_gcpb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x20))
#define rw_tbpb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x22))
#define rw_rapb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x24))
#define rw_capb_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x26))

#define rw_hrab_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x28))
#define rw_gcab_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x2a))
#define rw_raab_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x2c))
#define rw_htcq_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x2e))
#define rw_gccq_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x30))
#define rw_ercq_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x32))
#define rw_dmcq_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x34))
#define rw_smcq_num     (*(volatile word *)(BMU_REG_BASE + BUFFER_NUM_REG + 0x36))
/* TODO: wait update define -- end */

#define PROGRAM_TX_DONE_FLAG_REG_BIT_CNT 256
#define BUFFER_FLAG_REG 0x1c0
#define rd_nddn_bit(n)  (*(volatile dwrd *)(BMU_REG_BASE + BUFFER_FLAG_REG + (n)*4))
#define hal_bmu_clear_all_rd_nddn_bit   \
    for (uint8_t i = 0; i < 8; i++) {   \
        rd_nddn_bit(i) = 0xffffffff;    \
    }   \

#define hal_bmu_wait_all_rd_nddn_bit_set    \
    for (uint8_t i = 0; i < 8; i++) {   \
        while ((rd_nddn_bit(i) & 0xffffffff) != 0xffffffff);    \
    }   \

#endif
