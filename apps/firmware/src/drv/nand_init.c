#include <global.h>
#include <register.h>

#ifdef NAND_YX39070
#include "./micro_code_X39070"
#else
#include "./micro_code"
#endif

void fv_nand_init(void)
{
    word lw_cnt0, lw_cnt1, lw_cnt2;
    word lw_addr, lw_data;

/**************************        controller initial        **************************/
    //for FPGA, change PHY sampling phase
#ifdef RTL_SIM
    rb_nand_phs = NEG;
#else
    rb_nand_phs = POS;
#endif

    //meta/parity mode
    rb_cmd_mtsz = META_MODE;
    rb_cmd_ptsz = LDPC_MODE;

#ifdef NAND_YX39070
	rd_frag_end = (1 << 6) | (FRAG_QNTY - 1);
    rb_cmd_tp00 = 0x86;
    rb_cmd_tp10 = 0x8c;
	//rb_cmd_tp01 = 1;
	//rb_cmd_tp11 = 1;
#endif

    //forc FPGA, change cmd_add timeout threshold
    rd_tout_thr = 0x1000;

    //set Cadence PHY register
#ifdef CADENCE_PHY
  #ifdef ONFI_MODE
    rb_opr_mode = 0x2;               //set opr_mode as 2
    rb_index = 0x08;                 //set reg_index as all channel
    rb_dfi_rwmd = 0x0;               //set dfi_rd_mode & dfi_wr_mode as 0
  #endif
    rw_dll_rstn = 0xffff;            //release DLL reset
    while(rw_init_cpl != 0xffff);    //read initial status
#endif

    //set NIC initial register
#if (defined(ONFI_MODE))
  #if (defined(NAND_2400M))
    rb_tcr     = 11;
    rb_tcs     = 23;
    rb_tcs1    = 35;
    rb_tcd     = 119;
    rb_tcsd    = 11;
    rb_tceh    = 23;
    rb_tclr    = 11;
    rb_tar     = 11;
    rb_tcals2  = 29;
    rb_twhr    = 95;
    rb_twc     = 29;
    rb_twh     = 13;
    rb_tch     = 5;
    rb_tadl    = 179;
    rb_trhw    = 119;
    rb_tcdqss  = 35;
    rb_trpre   = 21;
    rb_twpre   = 17;
    rb_tcals   = 17;
    rb_tcalh   = 5;
    rb_twp     = 13;
    rb_tcas    = 5;
    rb_tdqsrh  = 5;
    rb_trpst   = 31;
    rb_trpsth  = 17;
    rb_tchz    = 35;
    rb_tclhz   = 35;
    rb_twpst   = 7;
    rb_twpsth  = 29;
    rb_tcdqsh  = 119;
    rb_tcres   = 5;
    rb_tdbs    = 5;
  #elif (defined(NAND_1600M))
    rb_tcr     = 7;
    rb_tcs     = 15;
    rb_tcs1    = 23;
    rb_tcd     = 79;
    rb_tcsd    = 7;
    rb_tceh    = 15;
    rb_tclr    = 7;
    rb_tar     = 7;
    rb_tcals2  = 19;
    rb_twhr    = 63;
    rb_twc     = 19;
    rb_twh     = 8;
    rb_tch     = 3;
    rb_tadl    = 119;
    rb_trhw    = 79;
    rb_tcdqss  = 23;
    rb_trpre   = 14;
    rb_twpre   = 11;
    rb_tcals   = 11;
    rb_tcalh   = 3;
    rb_twp     = 8;
    rb_tcas    = 3;
    rb_tdqsrh  = 3;
    rb_trpst   = 20;
    rb_trpsth  = 11;
    rb_tchz    = 23;
    rb_tclhz   = 23;
    rb_twpst   = 4;
    rb_twpsth  = 19;
    rb_tcdqsh  = 79;
    rb_tcres   = 3;
    rb_tdbs    = 3;
  #elif (defined(NAND_500M)) //500M for ku15p
    rb_tcr     = 2;
    rb_tcs     = 4;
    rb_tcs1    = 7;
    rb_tcd     = 24;
    rb_tcsd    = 2;
    rb_tceh    = 4;
    rb_tclr    = 2;
    rb_tar     = 2;
    rb_tcals2  = 6;
    rb_twhr    = 19;
    rb_twc     = 6;
    rb_twh     = 2;
    rb_tch     = 1;
    rb_tadl    = 37;
    rb_trhw    = 24;
    rb_tcdqss  = 7;
    rb_trpre   = 4;
    rb_twpre   = 3;
    rb_tcals   = 3;
    rb_tcalh   = 1;
    rb_twp     = 2;
    rb_tcas    = 1;
    rb_tdqsrh  = 1;
    rb_trpst   = 6;
    rb_trpsth  = 3;
    rb_tchz    = 7;
    rb_tclhz   = 7;
    rb_twpst   = 1;
    rb_twpsth  = 6;
    rb_tcdqsh  = 24;
    rb_tcres   = 1;
    rb_tdbs    = 1;
  #endif
    rb_tchk_en = 0xf;

    rb_nand_md = 0x0;                    //set nand mode as ddr mode
#elif (defined(CADENCE_PHY))
    rb_nand_md = LEGACY_MD | CADENCE_MD; //set nand mode as cadence mode
#else
    rb_nand_md = LEGACY_MD;              //set nand mode as legacy mode
#endif

#ifdef NAND_BICS5
    rb_nand_md = rb_nand_md | KIOXIA_MD;
#endif

    rb_inst_share = INST_SHARE;      //share instruction SRAM

    rb_frag_sft  = FRAG_SHIFT;
    rb_chan_sft  = CHAN_SHIFT;
    rb_ce_sft    = PCE_SHIFT;
    rb_lunce_sft = BANK_SHIFT;
    rb_page_sft  = PAGE_SHIFT;
    rb_blk_sft   = BLK_SHIFT;
    rb_plane_sft = PLANE_SHIFT;
    rb_plane_num = PLANE_QNTY;
    rw_mapu_size = MAPU_SIZE;
    rw_pg_size   = PG_SIZE;
    rd_phy_ce0   = 0x76543210;
    //rd_phy_ce0   = 0x76503214;
    rd_phy_ce1   = 0xfedcba98;
    rb_sts_cfg   = 0x0;
//    rb_fail_mask = 0x0;
    rw_tmr_data0 = 0x8;

    rb_mapu_sft_h = 2;
    rb_plan_sft_h = PLANE_SHIFT;
    rb_chan_sft_h = CHAN_SHIFT;
    rb_lnce_sft_h = BANK_SHIFT;
    rb_page_sft_h = PAGE_SHIFT;
    rb_cmpr_sft_h = (CMPR_SHIFT != 0) ? 1 : 0;
    rb_mapu_end_h = 3;
    rb_plan_end_h = PLANE_QNTY - 1;
    rb_chan_end_h = (1 << 6) | ((CHAN_QNTY - RAID_PNUM - 1) << 3) | (CHAN_QNTY - 1);
    rb_lnce_end_h = BANK_QNTY - 1;
    rw_page_end_h = PAGE_QNTY - 1;

#if (!defined(SLC_MODE) && !(defined(NAND_BICS5) || defined(NAND_YX39070)))
    rw_pgrp_tbl(0) = (0 << 14) | 3;
    rw_pgrp_tbl(1) = (2 << 14) | 1047;
    rw_pgrp_tbl(2) = (1 << 14) | 1063;
    rw_pgrp_tbl(3) = (2 << 14) | 2107;
    rw_pgrp_tbl(4) = (0 << 14) | PAGE_QNTY;
#else
    rw_pgrp_tbl(0) = (2 << 14) | PAGE_QNTY;
#endif

    rw_pext_num  = 4 * CMPR_NUM;
    rb_pstr_bmu  = FRAG_SHIFT + CHAN_SHIFT + BANK_SHIFT;
    rb_pstr_ecc  = FRAG_SHIFT + CHAN_SHIFT + BANK_SHIFT;
    rb_psft_bmu  = PAGE_SHIFT;
    rb_psft_ecc  = PAGE_SHIFT;

    rb_index     = 0x08;             //set reg_index as all channel
    rb_lunce_cfg = 0x10 | LUNCE_END; //set reg_lunce_ps & reg_lence_end for nand program


    //set micro code SRAM
    for(lw_cnt0 = 0; lw_cnt0 < MICRO_CODE_NUM; lw_cnt0++)
    {
        lw_addr = (lw_cnt0 << 9);
        lw_cnt1 = 0;
        do
        {
            lw_data = gw_micro_code[lw_cnt0][lw_cnt1];
            lw_cnt1++;
            for(lw_cnt2 = 0; lw_cnt2 < (8 >> INST_SHARE); lw_cnt2++)
            {
                rw_inst_mem(lw_addr) = lw_data;
                lw_addr++;
            }
        } while(lw_data != INST_FIN);
    }


/**************************          NAND initial          **************************/
    //reset NAND
    for(lw_cnt0 = 0; lw_cnt0 < CHAN_QNTY; lw_cnt0++)
    {
        rb_index = lw_cnt0;                               //set chan
        rb_set_cfg = CMD_HOLD;                            //set cmd hold
        rb_bus_mirr = 0xaa;
        for(lw_cnt1 = 0; lw_cnt1 < PCE_QNTY; lw_cnt1++)
        {
            rb_cmd_lunce = lw_cnt1;
            rb_nand_cen = rb_phy_ce;                      //set CEn
            //rb_nand_cmd = 0xFD;
            rb_nand_cmd = 0xFF;                           //reset cmd
#ifndef SKIP_STS_CHK
            rb_set_busy = BUSY_PS | FRC_RDS;              //check ready/busy
#endif
        }
    }

    //wait reset ready
#ifdef SKIP_STS_CHK
    for(lw_cnt0 = 0; lw_cnt0 < 1000; lw_cnt0++)
    {
  #ifdef RTL_SIM
      for(lw_cnt1 = 0; lw_cnt1 < 1; lw_cnt1++);
  #else
      for(lw_cnt1 = 0; lw_cnt1 < 1000; lw_cnt1++);
  #endif
    }
#else
    for(lw_cnt0 = 0; lw_cnt0 < CHAN_QNTY; lw_cnt0++)
    {
        rb_index = lw_cnt0;                               //set chan
        rb_set_cfg = 0;
        for(lw_cnt1 = 0; lw_cnt1 < PCE_QNTY; lw_cnt1++)
        {
            while((rw_sts_rdy & (1 << lw_cnt1)) == 0);
            if(rw_sts_fail & (1 << lw_cnt1))              //bad die
                fv_uart_print("reset fail: chan:%x, physical ce:%x\r\n", lw_cnt0, lw_cnt1);
        }
    }
#endif

#ifdef ONFI_MODE
    //set NAND timing mode
  #ifdef CADENCE_PHY
    //reset DLL
    rw_dll_rstn = (word)(~CHAN_MAP);                      //set DLL reset

    for(lw_cnt0 = 0; lw_cnt0 < CHAN_QNTY; lw_cnt0++)
    {
        rd_nphy_ctrl(lw_cnt0) = 0x104000;                 //set Cadence PHY parameter
        rd_nphy_tsel(lw_cnt0) = 0x0;
        rd_nphy_dqt(lw_cnt0)  = 0x2;
        rd_nphy_dqst(lw_cnt0) = 0x4;
        rd_nphy_lpbk(lw_cnt0) = 0x80f80002;
        rd_nphy_dllm(lw_cnt0) = 0x140004;
        rd_nphy_dlls(lw_cnt0) = 0x3f3f;
    }

    rw_dll_rstn = WORD_MASK;                              //release DLL reset
    while((rw_init_cpl & CHAN_MAP) != CHAN_MAP);          //read initial status
  #endif


    //select slow nand clk
    rb_nand_clk = CLK31M;


  #ifdef NAND_DIFF

    //set NAND NV-DDR3 config
    for(lw_cnt0 = 0; lw_cnt0 < CHAN_QNTY; lw_cnt0++)
    {
        rb_index = lw_cnt0;                               //set chan
        rb_set_cfg = CMD_HOLD;                            //set cmd hold
        for(lw_cnt1 = 0; lw_cnt1 < PCE_QNTY; lw_cnt1++)
        {
            rb_cmd_lunce = lw_cnt1;
            rb_nand_cen = rb_phy_ce;                      //set CEn
            rb_nand_cmd = 0xEF;                           //set feature cmd
            rb_nand_adr = 0x02;                           //set NV-DDR3 config reg
            lw_cnt2 = 150;
            while(--lw_cnt2!=0);                          //wait tADL of set feature
            rw_nand_wdt = 0x0707;                         //set P1 data
            rw_nand_wdt = 0x0;                            //set P2 data
            rw_nand_wdt = 0x0;                            //set P3 data
            rw_nand_wdt = 0x0;                            //set P4 data
            rb_set_busy = BUSY_PS | FRC_RDS;              //check ready/busy
        }
        rb_nand_cen = 0xff;                               //turn off CEn
    }

    //wait "set feature" ready
    for(lw_cnt0 = 0; lw_cnt0 < CHAN_QNTY; lw_cnt0++)
    {
        rb_index = lw_cnt0;                               //set chan
        rb_set_cfg = 0;
        while(rw_sts_rdy != 0xffff);
    }

  #endif


  #ifdef NAND_HSPD

  #ifndef NAND_BICS5 //BICS5 don't need to set timing mode
    //set NAND timing mode
    for(lw_cnt0 = 0; lw_cnt0 < CHAN_QNTY; lw_cnt0++)
    {
        rb_index = lw_cnt0;                               //set chan
        rb_set_cfg = CMD_HOLD;                            //set cmd hold
        for(lw_cnt1 = 0; lw_cnt1 < PCE_QNTY; lw_cnt1++)
        {
            rb_cmd_lunce = lw_cnt1;
            rb_nand_cen = rb_phy_ce;                      //set CEn
            rb_nand_cmd = 0xEF;                           //set feature cmd
            rb_nand_adr = 0x01;                           //set timing mode reg
            lw_cnt2 = 150;
            while(--lw_cnt2!=0);                          //wait tADL of set feature
    #ifdef NAND_B27B
            rw_nand_wdt = 0x3c3c;                         //set P1 data
    #else
            rw_nand_wdt = 0x3f3f;                         //set P1 data
    #endif
            rw_nand_wdt = 0x0;                            //set P2 data
            rw_nand_wdt = 0x0;                            //set P3 data
            rw_nand_wdt = 0x0;                            //set P4 data
            rb_set_busy = BUSY_PS | FRC_RDS;              //check ready/busy
        }
        rb_nand_cen = 0xff;                               //turn off CEn
    }

    //wait "set feature" ready
    for(lw_cnt0 = 0; lw_cnt0 < CHAN_QNTY; lw_cnt0++)
    {
        rb_index = lw_cnt0;                               //set chan
        rb_set_cfg = 0;
        while(rw_sts_rdy != 0xffff);
    }
  #endif

    //select fast nand clk
    rb_nand_clk = NAND_CLK;

  #endif


  #ifdef CADENCE_PHY
    //reset DLL
    rw_dll_rstn = (word)(~CHAN_MAP);                      //set DLL reset

    for(lw_cnt0 = 0; lw_cnt0 < CHAN_QNTY; lw_cnt0++)
    {
        rd_nphy_ctrl(lw_cnt0) = 0x4000;                   //set Cadence PHY parameter
        rd_nphy_tsel(lw_cnt0) = 0x0;
        rd_nphy_dqt(lw_cnt0)  = 0x2;
        rd_nphy_dqst(lw_cnt0) = 0x4;
        rd_nphy_lpbk(lw_cnt0) = 0x80f80002;
        rd_nphy_dllm(lw_cnt0) = 0x140004;
        rd_nphy_dlls(lw_cnt0) = 0x3f3f;
    }

    rw_dll_rstn = WORD_MASK;                              //release DLL reset
    while((rw_init_cpl & CHAN_MAP) != CHAN_MAP);          //read initial status
  #endif

#endif


#ifdef NAND_B27B
    //set flag check functionality
    for(lw_cnt0 = 0; lw_cnt0 < CHAN_QNTY; lw_cnt0++)
    {
        rb_index = lw_cnt0;                               //set chan
        rb_set_cfg = CMD_HOLD;                            //set cmd hold
        for(lw_cnt1 = 0; lw_cnt1 < PCE_QNTY; lw_cnt1++)
        {
            rb_cmd_lunce = lw_cnt1;
            rb_nand_cen = rb_phy_ce;                      //set CEn
            rb_nand_cmd = 0xEF;                           //set feature cmd
            rb_nand_adr = 0xDF;                           //set flag check functionality reg
            lw_cnt2 = 150;
            while(--lw_cnt2!=0);                          //wait tADL of set feature
            rw_nand_wdt = 0x1010;                         //set P1 data
            rw_nand_wdt = 0x0;                            //set P2 data
            rw_nand_wdt = 0x0;                            //set P3 data
            rw_nand_wdt = 0x0;                            //set P4 data
            rb_set_busy = BUSY_PS | FRC_RDS;              //check ready/busy
        }
        rb_nand_cen = 0xff;                               //turn off CEn
    }

    //wait "set feature" ready
    for(lw_cnt0 = 0; lw_cnt0 < CHAN_QNTY; lw_cnt0++)
    {
        rb_index = lw_cnt0;                               //set chan
        rb_set_cfg = 0;
        while(rw_sts_rdy != 0xffff);
    }
#endif

    return;
}
