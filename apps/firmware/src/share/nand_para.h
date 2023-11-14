#ifndef _NAND_MGR_H
#define _NAND_MGR_H


#define SLC_MODE   0x1
#define TLC_MODE   0x2


#define INST_SHARE  3 /* Refer to nand ctl register manual: reg_inst_share: Micro processor instruction sram share mode */


/* Fixed value. Now and in the future, these values are the same for all NAND particles. */
#define P4K_OFFSET_SHIFT  2 /* A NAND page has 4 4K pages, numbered 0-3, occupying 2 BITs with an offset shift of 2 */
#define P4K_OFFSET_MASK  ((1 << P4K_OFFSET_SHIFT) - 1)
#define PAGE_NUM_IN_TLC_WL 3
#define TLC_PAGE_SIZE (16) /* KB */
#define L4K_OR_P4K_SIZE (4096)
#define P4K_MASK (L4K_OR_P4K_SIZE -1)
#define PLANE_SHIFT_FRAG  2
#define P4K_NUM_PER_PAGE  (P4K_NUM_IN_PAGE)
#define ONE_DIE_SLC_PROU_DATA_SIZE (L4K_OR_P4K_SIZE*P4K_NUM_IN_PROU_SLC)
#define ONE_DIE_SLC_PROU_MASK       (ONE_DIE_SLC_PROU_DATA_SIZE - 1)
#define MAX_PLANE_NUM (8)
#define L4K_OR_P4K_SHIFT (12)

#define CMD_MAX_WAIT_TIME (1 * 500)  /* 1s: hardware time unit is ns */

typedef enum
{
    // 38MB/tlc block,2plane/die
    KAIXIA_BICS5,
    // 38MB/tlc block,4plane/die
    KAIXIA_BICS6,
    // ?MB/tlc block,8plane/die
    KAIXIA_BICS8,
    // ?MB/tlc
    MIRCON_B47R,
    // ?MB/qlc
    YMTC_X3_6070,
    // ?MB/tlc block, multi lun
    YMTC_X3_9070_MT,
    // 62MB/tlc block,6plane/die
    YMTC_X3_9070,

    NAND_TYPE_MAX_NUM,
} NAND_TYPE;
#define NAND_TYPE_MAX NAND_TYPE_MAX_NUM

#if defined(NAND_BICS5)
#define READ_ID_ADDR_0 0x00
#define READ_ID_ADDR_1 0x40

/* ===== read status ===== */
#define READ_STS_70H_CPTR 0//g_cmd_ptr_info.read_sts_70h_cptr
#define READ_STS_71H_CPTR 8//g_cmd_ptr_info.read_sts_71h_cptr
#define READ_STS_73H_CPTR 9//g_cmd_ptr_info.read_sts_73h_cptr
#define READ_STS_78H_CPTR 10//g_cmd_ptr_info.read_sts_78h_cptr

/* ===== erase ===== */
//#define ERS_CPTR       //g_cmd_ptr_info.ers_cptr
#define ERS_SINGE_CPTR  11//g_cmd_ptr_info.ers_singe_cptr

/* ===== program ===== */
//#define PROG_CPTR            //g_cmd_ptr_info.prog_cptr
#define PROG_UPPER_CPTR      2//g_cmd_ptr_info.prog_upper_cptr
#define PROG_FINE_CPTR       2//g_cmd_ptr_info.prog_fine_cptr
#define PROG_FINE_UPPER_CPTR 2//g_cmd_ptr_info.prog_fine_upper_cptr
#define PROG_SLC_CPTR        12//g_cmd_ptr_info.prog_slc_cptr

#define PROG_SINGE_CPTR            13//g_cmd_ptr_info.prog_singe_cptr
#define PROG_SINGE_UPPER_CPTR      2//g_cmd_ptr_info.prog_singe_upper_cptr
#define PROG_SINGE_FINE_CPTR       2//g_cmd_ptr_info.prog_singe_fine_cptr
#define PROG_SINGE_FINE_UPPER_CPTR 2//g_cmd_ptr_info.prog_singe_fine_upper_cptr
#define PROG_SINGE_SLC_CPTR        14//g_cmd_ptr_info.prog_singe_slc_cptr

#define PROG_CACHE_CPTR            2//g_cmd_ptr_info.prog_cache_cptr
#define PROG_CACHE_UPPER_CPTR      2//g_cmd_ptr_info.prog_cache_upper_cptr
#define PROG_CACHE_FINE_CPTR       2//g_cmd_ptr_info.prog_cache_fine_cptr
#define PROG_CACHE_FINE_UPPER_CPTR 2//g_cmd_ptr_info.prog_cache_fine_upper_cptr
#define PROG_CACHE_SLC_CPTR        2//g_cmd_ptr_info.prog_cache_slc_cptr

#define PROG_CACHE_SINGE_CPTR            2//g_cmd_ptr_info.prog_cache_singe_cptr
#define PROG_CACHE_SINGE_UPPER_CPTR      2//g_cmd_ptr_info.prog_cache_singe_upper_cptr
#define PROG_CACHE_SINGE_FINE_CPTR       2//g_cmd_ptr_info.prog_cache_singe_fine_cptr
#define PROG_CACHE_SINGE_FINE_UPPER_CPTR 2//g_cmd_ptr_info.prog_cache_singe_fine_upper_cptr
#define PROG_CACHE_SINGE_SLC_CPTR        2//g_cmd_ptr_info.prog_cache_singe_slc_cptr

/* ===== read ===== */
#define READ_CMD_CPTR               3//g_cmd_ptr_info.read_cptr
#define READ_SLC_CPTR           15//g_cmd_ptr_info.read_slc_cptr
#define READ_DATA_CPTR          4//g_cmd_ptr_info.read_data_cptr
#define READ_CACHE_END_CPTR     3//g_cmd_ptr_info.read_cache_end_cptr
#define READ_PARA_PAGE_CPTR     3//g_cmd_ptr_info.read_para_page_cptr
#define DUMMY_READ_CPTR         3//g_cmd_ptr_info.dummy_read_cptr
#define DUMMY_READ_SLC_CPTR     3//g_cmd_ptr_info.dummy_read_slc_cptr
#define READ_PARTIAL_CPTR       3//g_cmd_ptr_info.read_partial_cptr
#define READ_PARTIAL_SINGE_CPTR 3//g_cmd_ptr_info.read_partial_singe_cptr

/* ===== other ===== */
#define ZQ_CAL_LONG_CPTR   8//g_cmd_ptr_info.zq_cal_long_cptr
#define ZQ_CAL_SHORT_CPTR  8//g_cmd_ptr_info.zq_cal_short_cptr
#define VOLUME_SELECT_CPTR 8//g_cmd_ptr_info.volume_select_cptr
#define PROG_SUSPEND_CPTR  8//g_cmd_ptr_info.prog_suspend_cptr
#define PROG_RESUME_CPTR   8//g_cmd_ptr_info.prog_resume_cptr
#define ERS_SUSPEND_CPTR   8//g_cmd_ptr_info.ers_suspend_cptr
#define ERS_RESUME_CPTR    8//g_cmd_ptr_info.ers_resume_cptr

#define DCC_TRAINING_CPTR           9//g_cmd_ptr_info.dcc_training_cptr
#define READ_DQ_TRAINING_CPTR       9//g_cmd_ptr_info.read_dq_training_cptr
#define WRITE_DQ_TRAINING_CPTR      9//g_cmd_ptr_info.write_dq_training_cptr
#define WRITE_DQ_TRAINING_READ_CPTR 9//g_cmd_ptr_info.write_dq_training_read_cptr

#elif defined(NAND_YX39070)
#define READ_ID_ADDR_0 0x00
#define READ_ID_ADDR_1 0x20

/* ===== read status ===== */
#define READ_STS_70H_CPTR 0//g_cmd_ptr_info.read_sts_70h_cptr
#define READ_STS_71H_CPTR 8//g_cmd_ptr_info.read_sts_71h_cptr
#define READ_STS_73H_CPTR 9//g_cmd_ptr_info.read_sts_73h_cptr
#define READ_STS_78H_CPTR 10//g_cmd_ptr_info.read_sts_78h_cptr

/* ===== erase ===== */
//#define ERS_CPTR       //g_cmd_ptr_info.ers_cptr
#define ERS_SINGE_CPTR  11//g_cmd_ptr_info.ers_singe_cptr

/* ===== program ===== */
//#define PROG_CPTR            //g_cmd_ptr_info.prog_cptr
#define PROG_UPPER_CPTR      2//g_cmd_ptr_info.prog_upper_cptr
#define PROG_FINE_CPTR       2//g_cmd_ptr_info.prog_fine_cptr
#define PROG_FINE_UPPER_CPTR 2//g_cmd_ptr_info.prog_fine_upper_cptr
#define PROG_SLC_CPTR        12//g_cmd_ptr_info.prog_slc_cptr

#define PROG_SINGE_CPTR            13//g_cmd_ptr_info.prog_singe_cptr
#define PROG_SINGE_UPPER_CPTR      14//g_cmd_ptr_info.prog_singe_upper_cptr
#define PROG_SINGE_FINE_CPTR       2//g_cmd_ptr_info.prog_singe_fine_cptr
#define PROG_SINGE_FINE_UPPER_CPTR 2//g_cmd_ptr_info.prog_singe_fine_upper_cptr
#define PROG_SINGE_SLC_CPTR        15//g_cmd_ptr_info.prog_singe_slc_cptr

#define PROG_CACHE_CPTR            2//g_cmd_ptr_info.prog_cache_cptr
#define PROG_CACHE_UPPER_CPTR      2//g_cmd_ptr_info.prog_cache_upper_cptr
#define PROG_CACHE_FINE_CPTR       2//g_cmd_ptr_info.prog_cache_fine_cptr
#define PROG_CACHE_FINE_UPPER_CPTR 2//g_cmd_ptr_info.prog_cache_fine_upper_cptr
#define PROG_CACHE_SLC_CPTR        2//g_cmd_ptr_info.prog_cache_slc_cptr

#define PROG_CACHE_SINGE_CPTR            2//g_cmd_ptr_info.prog_cache_singe_cptr
#define PROG_CACHE_SINGE_UPPER_CPTR      2//g_cmd_ptr_info.prog_cache_singe_upper_cptr
#define PROG_CACHE_SINGE_FINE_CPTR       2//g_cmd_ptr_info.prog_cache_singe_fine_cptr
#define PROG_CACHE_SINGE_FINE_UPPER_CPTR 2//g_cmd_ptr_info.prog_cache_singe_fine_upper_cptr
#define PROG_CACHE_SINGE_SLC_CPTR        2//g_cmd_ptr_info.prog_cache_singe_slc_cptr

/* ===== read ===== */
#define READ_CMD_CPTR               3//g_cmd_ptr_info.read_cptr
#define READ_SLC_CPTR           16//g_cmd_ptr_info.read_slc_cptr
#define READ_DATA_CPTR          4//g_cmd_ptr_info.read_data_cptr
#define READ_CACHE_END_CPTR     3//g_cmd_ptr_info.read_cache_end_cptr
#define READ_PARA_PAGE_CPTR     3//g_cmd_ptr_info.read_para_page_cptr
#define DUMMY_READ_CPTR         3//g_cmd_ptr_info.dummy_read_cptr
#define DUMMY_READ_SLC_CPTR     3//g_cmd_ptr_info.dummy_read_slc_cptr
#define READ_PARTIAL_CPTR       3//g_cmd_ptr_info.read_partial_cptr
#define READ_PARTIAL_SINGE_CPTR 3//g_cmd_ptr_info.read_partial_singe_cptr

/* ===== other ===== */
#define ZQ_CAL_LONG_CPTR   8//g_cmd_ptr_info.zq_cal_long_cptr
#define ZQ_CAL_SHORT_CPTR  8//g_cmd_ptr_info.zq_cal_short_cptr
#define VOLUME_SELECT_CPTR 8//g_cmd_ptr_info.volume_select_cptr
#define PROG_SUSPEND_CPTR  8//g_cmd_ptr_info.prog_suspend_cptr
#define PROG_RESUME_CPTR   8//g_cmd_ptr_info.prog_resume_cptr
#define ERS_SUSPEND_CPTR   8//g_cmd_ptr_info.ers_suspend_cptr
#define ERS_RESUME_CPTR    8//g_cmd_ptr_info.ers_resume_cptr

#define DCC_TRAINING_CPTR           9//g_cmd_ptr_info.dcc_training_cptr
#define READ_DQ_TRAINING_CPTR       9//g_cmd_ptr_info.read_dq_training_cptr
#define WRITE_DQ_TRAINING_CPTR      9//g_cmd_ptr_info.write_dq_training_cptr
#define WRITE_DQ_TRAINING_READ_CPTR 9//g_cmd_ptr_info.write_dq_training_read_cptr

#endif


#define TP0_READ_OTHER_CMD 0
#define TP0_PROG_ERASE_CMD 1

#define TP1_NON_DATA_CMD 0
#define TP1_RW_DATA_CMD  1

#endif
