#ifndef _HAL_NCTL_REGISTER_H
#define _HAL_NCTL_REGISTER_H

#include <stdint.h>

typedef union
{
    uint16_t whole;
    struct
    {
        uint16_t reg_cmd_ptr : 6;   // 1:ers_cptr 2:prog_cptr 3:read_cptr
        uint16_t : 2;
        uint16_t reg_cmd_pri : 1;   // 0x0:low_pri 0x1:high_pri
        uint16_t : 2;
        uint16_t reg_cmd_end : 1;   // 0x0:not_end 0x1:is_end; command end / program unit end, HW can auto set this signal, see 0x44 register
        uint16_t reg_cmd_dmap : 3;  // LBA to PBA direct mapping address
        uint16_t : 1;
    } ctrl;
} RW_CMD_TYPE; //0x00-01

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t host_meta_size : 3; // 0:8Bytes, 1:16Bytes, 2:32Bytes, 3:64Bytes, 4:128Bytes, 5:0Bytes
        uint8_t fw_meta_size: 3;    // 0:4Bytes, 1:8Bytes, 2:12Bytes, 3:16Bytes, 4:20Bytes, 5:24Bytes, 6:28Bytes, 7:32Bytes
        uint8_t : 2;
    } ctrl;
} RW_CMD_MTSZ; //0x0c

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t cmd_exe : 1;  // 1: cmd busy
        uint8_t queue_full : 1;  // 1: imply cmd queue full
        uint8_t : 6;
    } ctrl;
} RO_CMD_EXE;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t reg_fda_req : 1;   // enable dma, H/W would auto clear this bit
        uint8_t reg_fda_nwr : 1;   // 1: write dma, 0: read dma
        uint8_t reg_fda_chan : 3;   // dma channel number
        uint8_t reg_fda_bok  : 1;   // buffer ready, command finish successful
        uint8_t : 2;
    } ctrl;
} RW_CMD_FDA;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t reg_qswh_en : 1;   // queue switch enable
        uint8_t reg_rcah_en : 1;   // read cache mode enable
        uint8_t reg_rcah_allq : 1; // read cache mode for two priority queue
        uint8_t reg_wcah_en : 1;   // write cache mode enable
        uint8_t reg_aipr_en : 1;   // AIPR mode enable
        uint8_t reg_susp_en : 1;   // program/erase suspend enable
        uint8_t reg_wseq_en : 1;
        uint8_t reg_bsts_chk : 1;
    } ctrl;
} RW_HACC_FUN;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t reg_dmap_sft : 6;  // insert cmd_dmap location
        uint8_t reg_dmap_bit : 2;  // insert cmd_dmap bit number
    } ctrl;
} RW_DMAP_ST;

typedef union
{
    uint16_t whole;
    struct
    {
        uint16_t reg_frag_end : 6;   // 4KB number in program unit
        uint16_t reg_frag_en  : 1;   // reg_frag_en: 1: use reg_frag_end as frag end, 0: use reg_frag_sft as frag end
        uint16_t : 1;
        uint16_t reg_auto_wrend : 1;    // enable auto write cmd end function
        uint16_t reg_auto_rdend : 1;    // enable auto read cmd end function
        uint16_t reg_auto_cmpr : 1;     // auto compression
        uint16_t : 5;
    } ctrl;
} RW_HACC_ND;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t reg_cmd_timeout_en : 1; // auto cmd time out enable
        uint8_t reg_timeout_force : 1; // force timeout
        uint8_t : 6;
    } ctrl;
} RW_CMD_TIMEOUT;


// base start offset: 0x0
struct cmd_info_reg
{
    // offset: 0x0   type: RW
    RW_CMD_TYPE reg_cmd_type;

    // offset: 0x2   type: RW
    // BMU buffer pointer in buffer message. The buffer pointer is based on MAPU as a unit
    // 0x0 ~ 0x9ff: 2560MAPU, DRAM buffer range
    // 0xa00 ~ 0xbff: 512MAPU, SRAM buffer range
    uint16_t reg_bmsg_bfp;

    // offset: 0x4   type: RW
    // NAND Physical Addr, {Blk uint, Page, CE, Chan, Frag},
    // if the capacity is 16TB/32TB, H/W would auto insert 0x1[5:4] into reg_cmd_padr
    uint32_t reg_cmd_padr;

    // offset: 0x8   type: RW
    // RAID table index, 0x1ff imply disable raid function when cmd done, auto return to default value
    uint16_t reg_bmsg_rid;

    // offset: 0xA   type: RW
    // Force ECC port info: bit0:enable, bit1: port number when cmd done, auto return to default value
    uint8_t reg_bmsg_fci;

    // offset: 0xB   type: RW
    // ECC decoder output to BMU queue number, when cmd done, auto return to default value
    // 0:to Host queue
    // 1:to cpu queue(GC read)
    // 2:to cpu queue(ecc fail, hw internal queue)
    // 3:to dram queue(hw internal queue)
    // 4:to cpu queue(put into SRAM)
    // 5:to raid decoder queue
    // 6:to cpu queue(put into DRAM other address or FTL table)
    // 7:Non 4KB
    uint8_t reg_bmsg_que;

    // offset: 0xC   type: RW
    RW_CMD_MTSZ reg_cmd_mtsz;

    // offset: 0xD   type: RW
    // Parity data size in buffer message. See ECC design spec
    uint8_t reg_bmsg_ptsz;

    // offset: 0xE   type: RW
    // for nand program path: flag ptr, when transfer done, use this ptr to set bit flag. (bit flag in BMU register)
    // for nand read path: LDPC decoder index
    uint8_t reg_bmsg_lid;

    // offset: 0xF   type: RW
    // for nand program path: 0: keep data buffer, 1: release data buffer (if program DRAM other address or FTL table, must set 0)
    // for nand read path: LDPC index type: 0:input buffer ptr, 1: LDPC table ptr
    uint8_t reg_bmsg_ltp : 1;
    uint8_t : 7;

    // offset: 0x10   type: RW
    uint32_t reg_bmsg_cid[4];

    // offset: 0x20   type: RW
    // meta data of DRAM other address or FTL table
    uint64_t reg_bmsg_tmt;

    // offset: 0x28   type: RW
    // dram ptr for dram other address or FTL table
    uint16_t reg_bmsg_dmp : 11;
    uint16_t : 5;

    // offset: 0x2A   type: RW
    uint16_t reg_bmsg_rtp;

    // offset: 0x2C   type: RW
    // FW send info to micro processor, reserved for future function
    uint16_t reg_proc_info;

    // offset: 0x2E
    uint16_t rsvd_2e;

    // offset: 0x30   type: RW
    RW_CMD_FDA reg_cmd_fda;

    // offset: 0x31   type: RO
    RO_CMD_EXE reg_cmd_exe;

    // offset: 0x32-0x3f
    uint8_t rsvd_32[15];

    // offset: 0x40   type: RW
    RW_HACC_FUN reg_hacc_fun;

    // offset: 0x41   type: RW
    RW_DMAP_ST reg_dmap_st;

    // offset: 0x42   type: RW
    // for read data
    uint8_t reg_rdt_cptr : 6;
    uint8_t : 2;

    // offset: 0x43   type: RW
    // for cache read end
    uint8_t reg_rcah_cptr : 6;
    uint8_t : 2;

    // offset: 0x44-0x45   type: RW
    RW_HACC_ND reg_hacc_nd;

    // offset: 0x46
    uint8_t rsvd_46;

    //offset: 0x47   type: RW RO
    RW_CMD_TIMEOUT reg_cmd_timeout;

    // offset: 0x48   type:RW
    uint32_t reg_tout_thr;

    // offset: 0x4c-0x4f
    uint32_t rsvd_4c;

    // offset: 0x50   type:RW
    // index0:64bits mapping to cmd_ptr[5:0];   0: read cmd, other cmd    1: program cmd, erase cmd
    // index1:64bits mapping to cmd_ptr[5:0];   0: non data cmd type: erase cmd, other cmd(read status cmd, set feature cmd鈥�.)    1: data cmd type: read cmd, program cmd
    uint64_t reg_cmd_tp[2];

    // offset: 0x60   type:RW
    uint8_t reg_ladr_chk : 1;
    uint8_t : 7;

    // offset: 0x61-0x67
    uint8_t rsvd_61[7];

    // offset: 0x68   type:RW
    uint64_t reg_cmd_ladr : 34;
    uint64_t : 30;

    // offset: 0x70   type:RW
    // queue cmd cnt select
    uint8_t reg_qcnt_sel;

    // offset: 0x71
    uint8_t rsvd_71;

    // offset: 0x72   type:RW
    // queue cmd cnt
    uint16_t que_cnt : 12;
    uint16_t : 4;
};

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t reg_legacy_md : 1;  // NAND interface Legacy mode, before change this register, need set nand_cen = 0xff
        uint8_t reg_cadence_md : 1; // If ONFI mode & ASIC, set 1, for FPGA, set 0
        uint8_t reg_kioxia_md : 1; // Transfer page number to word line
        uint8_t : 5;
    } ctrl;
} RW_NAND_MD;

typedef union
{
    uint32_t lunce;
    struct
    {
        uint8_t logic_bank_0 : 4; // logic bank[x + 0] mapping to physical LUNCE number
        uint8_t logic_bank_1 : 4; // logic bank[x + 1] mapping to physical LUNCE number
    } ctrl[4];
} RW_PHY_LUNCE;

typedef union
{
    uint32_t lunce;
    struct
    {
        uint8_t logic_frag_0 : 4; // logic frag[x + 0] mapping to plane number
        uint8_t logic_frag_1 : 4; // logic frag[x + 1] mapping to plane number
    } ctrl[4];
} RW_FRAG_PLANE;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t multi : 1;      // multi LUN or AIPR, need read status
        uint8_t rbn_pin : 1;    // nand status from nand RBn pin
        uint8_t : 6;
    } ctrl;
} RW_STS_CFG;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t reg_bbt_req : 1; // type:RWC  BBRM build table process enable. When HW done, HW would auto clear this bit.
        uint8_t reg_bbt_clr : 1; // type:RW   0: add entry, 1: clear entry
        uint8_t bbt_err : 1;     // type:RO   Add entry error, reg_bbt_pblk = null
        uint8_t bbt_fail : 1;    // type:RO   add entry fail
        uint8_t bbt_owr : 1;     // type:RO   add entry over write
        uint8_t bbt_rpt : 1;     // type:RO   add entry repeat
        uint8_t bbt_noh : 1;     // type:RO   clear entry not hit
        uint8_t : 1;
    } ctrl;
} REG_BBRM;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t lunce_num : 4;
        uint8_t channel_num : 3;
        uint8_t : 1;
    } ctrl;
} RW_DIE_CFG;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t select_func : 5;    // BBRM bit[x] select of hash function
        uint8_t : 3;
    } ctrl;
} RW_BBRM_HASH;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t tdbs_en : 1;    // Enable tDBS timing constraint
        uint8_t trhw_en : 1;    // Enable tRHW timing constraint
        uint8_t twpsth_en : 1;  // Enable tWPSTH timing constraint
        uint8_t timing_en : 1;  // Set tWPSTH/tRPSTH timing condition
        uint8_t : 4;
    } ctrl;
} RW_TCHK_EN;

// base start offset: 0x100
struct nand_set_reg
{
    // offset: 0x00   type: RW
    // [3] = 0 : [2:0] is channel select | [3] = 1 : only for write, write all channel
    uint8_t reg_index : 4;
    uint8_t : 4;

    // offset: 0x01   type: RW
    //  SRAM read data delay cycle
    uint8_t reg_sram_dly : 2;
    uint8_t : 6;

    // offset: 0x02   type: RW
    RW_NAND_MD reg_nand_md;

    // offset: 0x03   type: RW
    // Micro processor instruction sram share mode
    uint8_t reg_inst_share : 2;
    uint8_t : 6;

    // offset: 0x04   type: RW
    // rag number setting, frag imply 4KB number in read unit
    uint8_t reg_frag_sft : 3;
    uint8_t : 5;

    // offset: 0x05   type: RW
    // channel number setting
    uint8_t reg_chan_sft : 2;
    uint8_t : 6;

    // offset: 0x06   type: RW
    // CE number setting
    uint8_t reg_ce_sft : 3;
    uint8_t : 5;

    // offset: 0x07   type: RW
    // {LUN, CE} number setting
    uint8_t reg_lunce_sft : 3;
    uint8_t : 5;

    // offset: 0x08   type: RW
    // Page number setting
    uint8_t reg_pg_stf : 4;
    uint8_t : 4;

    // offset: 0x09   type: RW
    // Block number setting
    uint8_t reg_blk_sft : 4;
    uint8_t : 4;

    // offset: 0x0A   type: RW
    // plane number setting
    uint8_t reg_plane_sft : 3;
    uint8_t : 5;

    // offset: 0x0B   type: RW
    // plane number
    uint8_t reg_plane_num : 5;
    uint8_t : 3;

    // offset: 0x0C   type: RW
    // Code Word(Mapping unit) size, (4096Bytes + Meta data + ECC parity) Bytes
    uint16_t reg_mapu_size : 13;
    uint16_t : 3;

    // offset: 0x0E   type: RW
    //  NAND page size, Micron B27B is 18560Bytes, B58R is 18352Bytes
    uint16_t reg_pg_size;

    // offset: 0x10   type: RO
    uint64_t : 64;

    // offset: 0x18   type: RW
    RW_PHY_LUNCE reg_phy_lunce[2];

    // offset: 0x20   type: RW
    RW_FRAG_PLANE reg_frag_plane[8];

    // offset: 0x40   type: RW
    RW_STS_CFG reg_sts_cfg;

    // offset: 0x41   type: RW
    // Micro code cmd_ptr for read status
    uint8_t reg_rds_cptr : 6;
    uint8_t : 2;

    // offset: 0x42
    // there are 4 sets of registers in 0x44~0x47, use reg_sts_sel to access different set.
    uint8_t reg_sts_sel : 2;
    uint8_t : 6;
    uint8_t : 8;

    // offset: 0x44   type: RW
    // The bit mask for NAND ready/busy status
    uint8_t reg_rds_mask;

    // offset: 0x45   type: RW
    // The bit data for NAND ready
    uint8_t reg_rds_info;

    // offset: 0x46   type: RW
    // The bit mask for NAND program/erase fail/ok status
    uint8_t reg_fail_mask;

    // offset: 0x47   type: RW
    // The bit data for NAND program/erase ok;
    // (status_data & reg_fail_mask) = reg_fail_info, imply NAND program/erase ok
    uint8_t reg_fail_info;

    // offset: 0x48   type: RW
    // timer unit (us), if system clock is 600MHz, set 600
    uint16_t reg_tmr_base;

    // offset: 0x4a
    uint16_t : 16;

    // offset: 0x4c   type: RO
    // Channel map for program/erase fail; The corresponding bit indicates an error in writing NAND to CH.
    uint8_t sts_fail;

    // offset: 0x4d   type: RW
    // Interrupt mask for program/erase fail
    uint8_t reg_sts_mask;

    // offset: 0x4e
    uint16_t : 16;

    // offset: 0x50   type: RW
    // nand RBn pin mapping to logic RBn bit map table
    uint16_t reg_log_rbn[8];

    // offset: 0x60   type: RW
    // NAND busy time threshold, when use timer mode to check whether need to read status, set this threshold register
    uint16_t reg_tmr_dat[16];

    // offset: 0x80
    REG_BBRM reg_bbrm;

    // offset: 0x81   type: RW
    // BBRM input die number
    RW_DIE_CFG reg_bbt_die;

    // offset: 0x82   type: RW
    // BBRM input logical block number
    uint16_t reg_bbt_lblk : 14;
    uint16_t : 2;

    // offset: 0x84   type: RW
    // BBRM remapping physical block number
    uint16_t reg_bbt_pblk : 14;
    uint16_t : 2;

    // offset: 0x86   type: RO
    // BBRM hit entry number
    uint16_t bbt_entry : 13;
    uint16_t : 3;

    // offset: 0x88   type: RW
    // Select register entry
    uint8_t reg_ext_sel : 5;
    uint8_t : 3;

    // offset: 0x89   type: RW
    // die number of register entry
    RW_DIE_CFG reg_ext_die;

    // offset: 0x8a   type: RW
    // logical block number of register entry
    uint16_t reg_ext_lblk : 14;
    uint16_t : 2;

    // offset: 0x8c   type: RW
    // physical block number of register entry, 0x3ff imply null
    uint16_t reg_ext_pblk : 14;
    uint16_t : 2;

    // offset: 0x8e
    uint16_t : 16;

    // offset: 0x90   type: RW
    // BBRM hash function select bit, set 31 imply null
    RW_BBRM_HASH reg_bbrm_hash[13];
    uint8_t : 8;
    uint16_t : 16;

    // offset: 0xA0   type: RW
    uint16_t reg_dma_len[8];

    // offset: 0xB0
    uint8_t rsvd_b0[16];

    // offset: 0xC0   type: RW
    uint8_t reg_tcr;

    // offset: 0xC1   type: RW
    uint8_t reg_tcs;

    // offset: 0xC2   type: RW
    uint8_t reg_tcs1;

    // offset: 0xC3   type: RW
    uint8_t reg_tcd;

    // offset: 0xC4   type: RW
    uint8_t reg_tcsd;

    // offset: 0xC5   type: RW
    uint8_t reg_tceh;

    // offset: 0xC6   type: RW
    uint8_t reg_tclr;

    // offset: 0xc7   type: RW
    uint8_t reg_tar;

    // offset: 0xc8   type: RW
    uint8_t reg_tcals2;

    // offset: 0xc9   type: RW
    uint8_t reg_twhr;

    // offset: 0xca   type: RW
    uint8_t reg_twc;

    // offset: 0xcb   type: RW
    uint8_t reg_twh;

    // offset: 0xcc   type: RW
    uint8_t reg_tch;

    // offset: 0xcd   type: RW
    uint8_t reg_tadl;

    // offset: 0xce   type: RW
    uint8_t reg_trhw;

    // offset: 0xcf   type: RW
    uint8_t reg_tcdqss;

    // offset: 0xd0   type: RW
    uint8_t reg_trpre;

    // offset: 0xd1   type: RW
    uint8_t reg_twpre;

    // offset: 0xd2   type: RW
    uint8_t reg_tcals;

    // offset: 0xd3   type: RW
    uint8_t reg_tcalh;

    // offset: 0xd4   type: RW
    uint8_t reg_twp;

    // offset: 0xd5   type: RW
    uint8_t reg_tcas;

    // offset: 0xd6   type: RW
    uint8_t reg_tdqsrh;

    // offset: 0xd7   type: RW
    uint8_t reg_trpst;

    // offset: 0xd8   type: RW
    uint8_t reg_trpsth;

    // offset: 0xd9   type: RW
    uint8_t reg_tchz;

    // offset: 0xda   type: RW
    uint8_t reg_tclhz;

    // offset: 0xdb   type: RW
    uint8_t reg_twpst;

    // offset: 0xdc   type: RW
    uint8_t reg_twpsth;

    // offset: 0xdd   type: RW
    uint8_t reg_tcdqsh;

    // // offset: 0xde   type: RW
    // uint8_t reg_tcres;

    // offset: 0xde   type: RW
    uint8_t reg_tdbs;

    // offset: 0xdf   type: RW
    RW_TCHK_EN reg_tchk_en;

    // offset: 0xe0    type: RW
    // legacy read setup time
    uint8_t reg_tlrs;

    // offset: 0xe1    type: RW
    // legacy read hold time
    uint8_t reg_tlrh;

    // offset: 0xe2    type: RW
    // legacy write setup time
    uint8_t reg_tlws;

    // offset: 0xe3    type: RW
    // legacy write hold time
    uint8_t reg_tlwh;

    // offset: 0xe4    type: RW
    // Legacy mode read latch time
    uint8_t reg_lrd_dly;

    // offset: 0xe5
    uint8_t rsvd_e5[11];

    /*=========Cadence PHY control register============*/
    // offset: 0xf0
    uint8_t phy_ctrl[256];
};

//  [7]=0, [6:0]=0, use reg_cmd_mapu(0x1041) as ECC unit number
//  [7]=0, [6:0]!=0, use [6:0] as ECC unit number
//  [7]=1 : [2:0] mean reg select, use reg_dma_len(0x1a0) as DMA length
typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t data : 7;   // mode data
        uint8_t flag : 1;   // flag of type select
    } ctrl;
} WO_ENA_DMA;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t read : 1;   // NAND read cache cmd
        uint8_t write : 1;  // NAND write cache cmd
        uint8_t : 6;
    } ctrl;
} RW_CMD_ECT;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t ready : 1;  //  NAND die ready, H/W use lunce to select the NAND ready
        uint8_t fail : 1;   // NAND die program/erase fail, H/W use lunce to select the NAND die
        uint8_t : 2;
        uint8_t rdy_sel : 2; // micro processor can use different 0x44/0x45 register sets to check whether NAND ready by change this select register(reg_rdy_sel)
        uint8_t fail_sel : 2; // micro processor can use different 0x46/0x47 register sets to check whether NAND program/erase fail by change this select register(reg_fail_sel).
    } ctrl;
} RO_DIE_STS;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t reg_tmr_sel : 4;    // Select 0x160~0x17F nand busy time threshold
        uint8_t reg_frc_rds : 1;    // For program/erase, force read status
        uint8_t reg_busy_ps : 1;    // write only, When write 1, H/W set nand status busy
        uint8_t reg_rds_hpri: 1;    // Set read_status cmd of micro_code as high priority
        uint8_t reg_rds_pause:1;    // pasue read_status cmd of micro_code
    } ctrl;
} RW_SET_BUSY;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t plane_cnt : 4;  // change row_addr(0x54) plane value
        uint8_t plane_sel : 1;  //  0: use reg_plane_cnt for row_addr(0x54), 1:use plane_cnt(0x50) for row_addr(0x54)
        uint8_t : 3;
    } ctrl;
} RW_PLANE_CFG;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t bbrm_en : 1;    // enable Bad Blk ReMap(bbrm) table
        uint8_t bbrm_exe : 1;   // bbrm execution, when remap finish, H/W would set this bit as 0
        uint8_t bbrm_rds: 1;    // enable bbrm for read status command
        uint8_t : 5;
    } ctrl;
} RW_BBRM_CFG;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t reg_dma_rdy : 1;    // type: RW   0: don鈥檛 return ready until H/W DMA done, 1: H/W return ready and FW need check 鈥渘and_dma_exe鈥�
        uint8_t nand_dma_exe : 1;   // type: RO   Read only, NAND DMA engine execution, when DMA done, H/W auto clear this bit
        uint8_t : 6;
    } ctrl;
} REG_DMA_CFG;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t msg_sel : 4;    // select reg_proc_r(R24 ~ R31), each lunce has an independent 8bytes register in R24~R31, total support 16 lunce
        uint8_t msg_auto : 1;   // use reg_cmd_lunce(0x4a) to select reg_proc_r, 0: use reg_msg_sel(0x70) to select reg_proc_r
        uint8_t : 3;
    } ctrl;
} RW_MSG_CFG;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t rst_mic_pro : 1;    // reset micro processor
        uint8_t rst_nand_func : 1;  // reset nand interface function
        uint8_t hold_mic_pro : 1;   // hold micro processor
        uint8_t : 5;
    } ctrl;
} RW_FUNC_EXCEPT;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t bus_keep_cle_ale : 1;   // bus_keep state, nand_cle, nand_ale
        uint8_t bus_keep_dqe_dqo : 1;   // bus_keep state, nand_dqe, nand_dqo
        uint8_t idle_cle_ale : 1;       // idle state, nand_cle, nand_ale
        uint8_t idle_dqe_dqo : 1;       // idle state, nand_dqe, nand_dqo
        uint8_t rwm_idle_ren: 1;        // rwm_idle state, nand_ren
        uint8_t rwm_idle_dqse_dqso : 1; // rwm_idle state, nand_dqse, nand_dqso
        uint8_t : 2;
    } ctrl;
} RW_BUS_KEEP;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t lunce_end : 4;  // lunce end, if 1 die, set 0, 2die, set 1鈥�
        uint8_t lunce_ps : 1;   // initial queue arbiter function
        uint8_t : 3;
    } ctrl;
} RW_LUNCE_CFG;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t die : 4;    // die select
        uint8_t pri : 1;    // priority select
        uint8_t : 3;
    } ctrl;
} RW_QPTR_SEL;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t die_pause : 4;  // suspend execution of one die command
        uint8_t all_pause : 1;  // suspend execution of all command
        uint8_t : 3;
    } ctrl;
} RW_PAUSE_CFG;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t low : 1;    // low queue command pause
        uint8_t high : 1;   // high queue command pause
        uint8_t : 6;
    } ctrl;
} RW_QUE_PAUSE;

typedef union
{
    uint32_t whole;
    struct
    {
        uint32_t cind_wen : 4;  // Byte write enable for indirect access
        uint32_t cind_req : 1;  // request for indirect access, HW auto clear
        uint32_t : 3;
        uint32_t cind_adr : 20; // address for indirect access
        uint32_t : 4;
    } ctrl;
} RW_CIND_CFG;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t cmd_act : 1;    // micro processor active
        uint8_t pcmd_req : 1;   // send next command request
        uint8_t nprd_full : 1;  // NAND Read FIFO full signal
        uint8_t npwr_empty : 1; // NAND write FIFO empty signal
        uint8_t rd_miss : 1;    // NAND鈥檚 REn, DQS pulse number is not equal
        uint8_t : 3;
    } ctrl;
} RO_CMD_STS;

typedef union
{
    uint8_t whole;
    struct
    {
        uint8_t rd_mode : 1;    // param_extended_rd_mode: Cadence ONFI PHY read mode control register
        uint8_t wr_mode : 1;    // param_extended_wr_mode: Cadence ONFI PHY write mode control register
        uint8_t : 6;
    } ctrl;
} RW_DFI_RWMD;

typedef union
{
    uint32_t whole;
    struct
    {
        uint32_t low : 16;  // low priority command queue valid, diff bit imply diff die
        uint32_t high : 16; // high priority command queue valid, diff bit imply diff die
    } ctrl;
} RO_CMDQ_VLD;

typedef union
{
    uint32_t whole;
    struct
    {
        uint32_t read : 16;     // read command ready, diff bit imply diff die
        uint32_t write : 16;    // write command ready, diff bit imply diff die
    } ctrl;
} RO_QUE_RDY;

// base start offset: 0x1000
struct channel_set_reg
{
    // offset: 0x0    type: WO
    // Read data from NAND interface; If write this register(any value), ptc_hdr would read data from NAND to 0x1058[15:0]
    uint16_t reg_nand_rdt;
    uint16_t : 16;

    // offset: 0x4    type: WO
    // Write data to NAND interface; If write this register, ptc_hdr would write data to NAND
    uint16_t reg_nand_wdt;
    uint16_t : 16;

    // offset: 0x8    type: WO
    // Enable H/W Read DMA engine
    WO_ENA_DMA reg_ena_rdma;
    uint8_t : 8;
    uint16_t : 16;

    // offset: 0xC    type: WO
    // Enable H/W Write DMA engine
    WO_ENA_DMA reg_ena_wdma;
    uint8_t : 8;
    uint16_t : 16;

    // offset: 0x10    type: WO
    // Send Command to NAND interface
    uint8_t reg_nand_cmd;
    uint8_t : 8;
    uint16_t : 16;

    // offset: 0x14    type: WO
    // Send Address to NAND interface
    uint8_t reg_nand_adr;
    uint8_t : 8;
    uint16_t : 16;

    // offset: 0x18    type: WO
    // [7:4]!=0: disable all CE;
    // [7:4]=0: [3:0] mean CE number, when enable CE, auto disable all other CE
    uint8_t reg_ena_cen;
    uint8_t : 8;
    uint16_t : 16;

    // offset: 0x1C    type: WO
    // Enable wait function
    uint8_t reg_ena_wait;
    uint8_t : 8;
    uint16_t : 16;

    // offset: 0x20    type: RO
    // micro processor internal register
    uint8_t reg_proc_r[32];

    // offset: 0x40    type: RW
    // When write this register, imply micro processor would execute this command
    uint8_t reg_cmd_ptr : 6;
    uint8_t : 2;

    // offset: 0x41    type: RW
    // DMA mapu number(4KB number)
    uint8_t reg_cmd_mapu : 7;
    uint8_t : 1;

    // offset: 0x42    type: RW
    // CPU send information to micro processor
    uint16_t reg_proc_info;

    // offset: 0x44    type: RW
    // {NAND Physical Blk number, Page Number}, before bad block remapping
    uint32_t reg_cmd_blkpg : 29;
    uint32_t : 3;

    // offset: 0x48    type: RW
    // H/W send info to micro processor
    RW_CMD_ECT reg_cmd_ect;

    // offset: 0x49    type: RW
    // mapu offset in program unit
    uint8_t reg_cmd_frag : 6;
    uint8_t : 2;

    // offset: 0x4A    type: RW
    // {LUN number, CE number}
    uint8_t reg_cmd_lunce : 4;
    uint8_t : 4;

    // offset: 0x4B    type: RW
    // current channel
    uint8_t chan_loc : 3;
    uint8_t : 5;

    // offset: 0x4C    type: RO
    // Current command CE after remapping
    uint8_t phy_ce : 4;
    uint8_t : 4;

    // offset: 0x4D    type: RO
    // Current command LUN after remapping
    uint8_t phy_lun : 3;
    uint8_t : 5;

    // offset: 0x4E    type: RO
    // one plane command
    uint8_t single_plane : 1;
    uint8_t : 7;

    // offset: 0x4F    type: RO
    RO_DIE_STS ro_die_sts;

    // offset: 0x50    type: RO
    // NAND DMA current plane number
    uint8_t plane_cnt : 4;
    uint8_t : 4;

    // offset: 0x51    type: RO
    // NAND DMA transfer mapu count
    uint8_t mapu_cnt : 6;
    uint8_t : 2;

    // offset: 0x52    type: RO
    // NAND DMA current column address in page
    uint16_t col_cnt;

    // offset: 0x54    type: RO
    // {Lun_num, Bad block remapping blk, current_plane, page_num}, H/W would auto calculate plane number
    uint32_t row_addr : 29;
    uint32_t : 3;

    // offset: 0x58    type: RO
    // NAND interface read data
    uint32_t nand_rdata;

    // offset: 0x5C    type: RWC
    // Die map for NAND ready, write 1 to set
    uint16_t status_rdy;

    // offset: 0x5E    type: RWC
    // Die map for NAND program/erase fail, write 1 to clear
    uint16_t status_fail;

    // offset: 0x60    type: RW
    RW_SET_BUSY reg_set_busy;

    // offset: 0x61    type: RW
    // NAND Read status data, When write this register, H/W would check the value and set status_rdy & status_fail
    uint8_t reg_rds_dat;

    // offset: 0x62    type: RW
    RW_PLANE_CFG reg_plane_cfg;

    // offset: 0x63    type: RW
    RW_BBRM_CFG reg_bbrm_cfg;

    // offset: 0x64
    REG_DMA_CFG reg_dma_cfg;

    // offset: 0x65    type: RW
    // set NAND DMA engine to pause
    uint8_t reg_dma_pause : 1;
    uint8_t : 7;

    // offset: 0x66    type: RW
    // micro processor pc jump number
    uint16_t reg_jump_num;

    // offset: 0x68    type: RW
    RW_MSG_CFG reg_msg_cfg;

    // offset: 0x69    type: RW
    RW_FUNC_EXCEPT reg_func_except;

    // offset: 0x6A    type: RW
    RW_BUS_KEEP reg_bus_keep;

    // offset: 0x6B    type: RW
    uint8_t reg_bus_mirr;

    // offset: 0x6C    type: RW
    RW_LUNCE_CFG reg_lunce_cfg;

    // offset: 0x6D    type: RW
    RW_QPTR_SEL reg_qptr_sel;

    // offset: 0x6E    type: RW
    RW_PAUSE_CFG reg_pause_cfg;

    // offset: 0x6F    type: RW
    RW_QUE_PAUSE reg_que_pause;

    // offset: 0x70    type: RW
    RW_CIND_CFG reg_cind_cfg;

    // offset: 0x74    type: RW
    // read/write data for indirect access
    uint32_t reg_cind_dat;

    // offset: 0x78    type: RO
    // micro processor program count location
    uint16_t inst_adr : 15;
    uint16_t : 1;

    // offset: 0x7A    type: RO
    // NAND cmd sequence state machine
    uint8_t ptc_mode : 5;
    uint8_t : 3;

    // offset: 0x7B    type: RO
    // NAND onfi Read/Write state machine
    uint8_t nrw_mode : 4;
    uint8_t : 4;

    // offset: 0x7C    type: RO
    RO_CMD_STS ro_cmd_sts;

    // offset: 0x7D    type: RW
    RW_DFI_RWMD reg_dfi_rwmd;

    // offset: 0x7E    type: RW
    // NAND IO pad config register
    uint16_t io_pad_cfg;

    // offset: 0x80    type: RO
    RO_CMDQ_VLD ro_cmdq_vld;

    // offset: 0x84    type: RO
    // command queue read pointer
    uint16_t que_rptr : 12;
    uint16_t : 4;

    // offset: 0x86    type: RO
    // command queue write pointer
    uint16_t que_wptr : 12;
    uint16_t : 4;

    // offset: 0x88    type: RO
    RO_QUE_RDY ro_que_rdy;

    // offset: 0x8C    type: RO
    // queue arbiter function state machine
    uint8_t que_abt_st : 3;
    uint8_t : 5;
    uint8_t : 8;

    // offset: 0x8E    type: RO
    // ONFI PHY NAND CEn
    uint8_t nand_cen;

    // offset: 0x8F    type: RO
    // ONFI PHY NAND RBn
    uint8_t nphy_rbn;

    // offset: 0x90
    uint8_t rsvd_90[7];

    // offset: 0x97    type: RW
    // select keep_blkpg(0x1098)
    uint8_t reg_keep_sel : 4;
    uint8_t : 4;

    // offset: 0x98    type: RO
    // when program/erase finish, use reg_cmd_blkpg(0x1044) to keep logic blk/page per die, for program/erase error handling
    uint32_t keep_blkpg : 29;
    uint8_t : 3;
};

// SRAM access
// #define rd_cmdq_mem(n)   (*(volatile uint32_t *)(NAND_REG_BASE + 0x40000 + ((n) << 2)))
// #define rw_inst_mem(n)   (*(volatile uint16_t *)(NAND_REG_BASE + 0x80000 + ((n) << 1)))
// #define rd_bbrm_mem(n)   (*(volatile uint32_t *)(NAND_REG_BASE + 0xC0000 + ((n) << 2)))

// g_register
#define g_cmd_info_reg ((volatile struct cmd_info_reg*)(NAND_REG_BASE + 0x0))
#define g_nand_set_reg ((volatile struct nand_set_reg*)(NAND_REG_BASE + 0x100))
#define g_channel_set_reg ((volatile struct channel_set_reg*)(NAND_REG_BASE + 0x1000))

#define rw_bmsg_fci      (*(volatile word *)(NAND_REG_BASE+0x0a))
#define rb_cmd_tp00      (*(volatile byte *)(NAND_REG_BASE+0x50)) //0: read cmd, other cmd, 1: program cmd, erase cmd
#define rb_cmd_tp01      (*(volatile byte *)(NAND_REG_BASE+0x51)) //0: read cmd, other cmd, 1: program cmd, erase cmd
#define rb_cmd_tp02      (*(volatile byte *)(NAND_REG_BASE+0x52)) //0: read cmd, other cmd, 1: program cmd, erase cmd

#define rb_cmd_tp10      (*(volatile byte *)(NAND_REG_BASE+0x58)) //0: non data cmd type: erase cmd, other cmd(read status cmd, set feature cmd…..), 1: data cmd type: read cmd, program cmd
#define rb_cmd_tp11      (*(volatile byte *)(NAND_REG_BASE+0x59)) //0: non data cmd type: erase cmd, other cmd(read status cmd, set feature cmd…..), 1: data cmd type: read cmd, program cmd
#define rb_cmd_tp12      (*(volatile byte *)(NAND_REG_BASE+0x5a)) //0: non data cmd type: erase cmd, other cmd(read status cmd, set feature cmd…..), 1: data cmd type: read cmd, program cmd

// ECC Register
#define rb_rdmz_en       (*(volatile uint8_t  *)(ECC_REG_BASE + 0x3C4)) // randomizer enable
#define rb_rdmz_fsz      (*(volatile uint8_t  *)(ECC_REG_BASE + 0x3C5)) // frame size, 0:512B, 1:1KB, 2: 2KB, 3:4KB
#define rb_rdmz_psz      (*(volatile uint8_t  *)(ECC_REG_BASE + 0x3C6)) // page size, 0:4KB, 1:8KB, 2:16KB, 3:32KB
#define rb_rdmz_mode     (*(volatile uint8_t  *)(ECC_REG_BASE + 0x3C7)) // nand flash vendor mode, 0:traditional kioxia/ymtc, 1: micron/hynix, 2: samsung, 3:kioxia bics
#define rw_rdmz_seed     (*(volatile uint32_t *)(ECC_REG_BASE + 0x3C8)) // initial seed
#define rb_rdmz_pe       (*(volatile uint8_t  *)(ECC_REG_BASE + 0x3CC)) // nand pe cycle, for kioxia bics only
#define rb_rdmz_mapo     (*(volatile uint8_t  *)(ECC_REG_BASE + 0x3CD)) // remap offset, for kioxia bics only

#endif // _NAND_ST_REGISTER_H
