#ifndef _ERROR_CODE_H
#define _ERROR_CODE_H

#include "../hal/hal_err_code.h"

// calculate err code first index
#define ERROR_CODE_CAL(idx) (((idx) << (8)) + 1)

/* ============ module define ============ */
enum MODULE_ID
{
    MODULE_BMU = 0x1,
    MODULE_FTL,
    MODULE_NCTL,
    MODULE_BLK_CTL,
};

/* ============ error code define ============ */
enum BMU_ERR_CODE
{
    BMU_ERR_CODE0 = ERROR_CODE_CAL(MODULE_BMU),                 // 0x101: code0
};

enum FTL_ERR_CODE
{
    FTL_ERR_CODE0 = ERROR_CODE_CAL(MODULE_FTL),                 // 0x201: code0
};

enum NCTL_ERR_CODE{
    NCTL_CHECK_STATUS_BAD_BLOCK = ERROR_CODE_CAL(MODULE_NCTL),  // 0x301: bad block
    NCTL_CHECK_STATUS_UNC0_ECC_ERR,                             // 0x302: unc0 ECC check fail
    NCTL_CHECK_STATUS_UNC1_ECC_ERR,                             // 0x303: unc1 ECC check fail
    NCTL_CHECK_STATUS_NAND_PE_ERR,                              // 0x304: NAND program/erase err
    NCTL_CHECK_STATUS_CHANNEL_MAP_ERR,                          // 0x305: Channel map for program/erase fail
    NCTL_CHECK_STATUS_MASK_PE_ERR,                              // 0x306: Interrupt mask for program/erase fail
    NCTL_CHECK_STATUS_NAND_DIE_PE_ERR,                          // 0x307: NAND die program/erase fail
    NCTL_CHECK_STATUS_NAND_STATUS_ERR,                          // 0x308: NAND Read status data
    NCTL_READ_RETRY_FEATUER_SET_OUT_RANGE,                      // 0x309: Chose index out of config range
    NCTL_READ_RETRY_FEATUER_SET_FAIL,                           // 0x30A: Get lun feature cfg not equal set
    NCTL_READ_RETRY_ALL_TRY_FAIL_TLC,                           // 0x30B: tlc: all cfg retry read is failed
    NCTL_READ_RETRY_ALL_TRY_FAIL_SLC,                           // 0x30C: slc: all cfg retry read is failed
    NCTL_READ_RETRY_FAIL_NOT_SUPPOR_CMD,                        // 0x30D: cmd not support to read rerty
    NCTL_WAIT_LOOP_TIME_OUT,                                    // 0x30E: wait time is out range
    NCTL_READ_STATUS_ERASE_FAIL,                                // 0x30F: block erase bit fail
    NCTL_READ_STATUS_PROGRAM_FAIL,                              // 0x310: program bit fail
    NCTL_READ_STATUS_CURRENT_PROGRAM_FAIL,                      // 0x311: current program bit fail
    NCTL_READ_STATUS_PREVIOUUS_PROGRAM_FAIL,                    // 0x312: previous program bit fail
    NCTL_READ_STATUS_DCC_TRAINING_FAIL,                         // 0x313: dcc training bit fail
    NCTL_READ_STATUS_ZQ_CALIBRATION_FAIL,                       // 0x314: zq calibration bit fail
    NCTL_READ_STATUS_PLANE0_FAIL,                               // 0x315: plane#0 bit fail
    NCTL_READ_STATUS_PLANE1_FAIL,                               // 0x316: plane#1 bit fail
    NCTL_READ_STATUS_PLANE2_FAIL,                               // 0x317: plane#2 bit fail
    NCTL_READ_STATUS_PLANE3_FAIL,                               // 0x318: plane#3 bit fail
    NCTL_READ_STATUS_UN_SUPPORT_STATUS_CMD,                     // 0x319: not support read status commond
    NCTL_READ_STATUS_UN_SUPPORT_LAST_SEND_CMD,                  // 0x31A: not support of last send commond
    NCTL_READ_STATUS_ESR_ERR,                                   // 0x31B: Error event from ESR[7:0]
    NCTL_READ_STATUS_SUSPEND_ERASE_FAIL,                        // 0x31C: suspend block erase bit fail
    NCTL_READ_STATUS_SUSPEND_PROGRAM_FAIL,                      // 0x31D: suspend program bit fail
    NCTL_READ_STATUS_PLANE4_FAIL,                               // 0x31E: plane#4 bit fail
    NCTL_READ_STATUS_POWER_DROOP_DETECTION,                     // 0x31F: error event of power droop detetion
    NCTL_READ_STATUS_SUSPEND_PRE_PRO_FAIL,                      // 0x320: suspend previous program bit fail
    NCTL_READ_4K_CMD_ERR,                                       // 0x321: read cmd find err
};

enum BLK_CTL_ERR_CODE {
    BLK_CTL_STATUS_BAD_BLOCK = ERROR_CODE_CAL(MODULE_BLK_CTL),  // 0x401: block is bad block
    BLK_CTL_GET_NEW_NODE_FAIL,                                  // 0x402: array list get new node fail
    BLK_CTL_WAIT_LOOP_TIME_OUT,                                 // 0x403: wait time is out range
    BLK_CTL_REMAP_NOT_HAS_FREE_SEL,                             // 0x404: ext remap entry is all used
    BLK_CTL_REMAP_ADD_ERR_PBLK_NULL,                            // 0x405: add entry error, pblk is null
    BLK_CTL_REMAP_ADD_ERR_ADD_FAIL,                             // 0x405: add entry fail
    BLK_CTL_REMAP_ADD_ERR_OVER_WRITE,                           // 0x406: add entry over write(The remap source is the same twice, but the destination is different)
    BLK_CTL_REMAP_ADD_ERR_REPEAT,                               // 0x407: add entry repeat(Both remaps have the same source and destination)
    BLK_CTL_REMAP_CLAER_NOT_HIT,                                // 0x408: clear entry not hit
    BLK_CTL_REMAP_LIST_CAN_NOT_GET_NEW,                         // 0x409: remap list can't get new node
    BLK_CTL_REMAP_LIST_INDEX_GET_FAIL,                          // 0x40A: remap list can't select index node
    BLK_CTL_REMAP_LIST_FIND_SAME_PBLK,                          // 0x40B: remap list find same pblk
    BLK_CTL_REMAP_LIST_GET_FREE_BLOCK_FAIL,                     // 0x40C: remap list get free blk fail
    BLK_CTL_REMAP_EXT_ALL_USED_FAIL,                            // 0x40D: remap ext list all used
    BLK_CTL_REMAP_EXT_CLAER_OVER_RANGE,                         // 0x40E: clear ext entry over max range
};

#endif
