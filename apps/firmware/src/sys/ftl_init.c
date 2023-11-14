#include <global.h>
#include <register.h>

dwrd *gd_l2p_tbl;
dwrd *gd_p2l_tbl;
dwrd *gd_vmap_tbl;
byte *gb_crl_tbl;
dwrd gd_vcnt_tbl[RBLK_QNTY];
dwrd gd_ecnt_tbl[RBLK_QNTY];
DBL_TBL gs_dbl_tbl[RBLK_QNTY];
byte gb_blk_type[RBLK_QNTY];

word gw_fblk_str;
word gw_fblk_end;
word gw_fblk_num;

byte gb_get_htblk;
byte gb_get_gcblk;

PHY_ADR gs_head_padr[HEAD_QNTY];

//p2l table message
dwrd gd_pmsg_pcf[PMSG_NUM]; //page, cech, frag
byte gb_pmsg_npg[PMSG_NUM]; //next page unit
byte gb_pmsg_epg[PMSG_NUM]; //end page unit

byte gb_p2lp_nrd[2];
byte gb_p2lp_cnt[2];
byte gb_gcad_wat[2];

word gw_chkz_cnt[8];
byte gb_chkz_fin[8];

word gw_bblk_str;
word gw_bblk_end;

word gw_null_ptr;

void fv_ers_build(void)
{
    byte lb_good_blk, lb_ce, lb_chan, lb_cech;
    word lw_blk_ptr;

#ifndef RTL_SIM
    //L2P Table init
    //dwrd ld_tmp_ptr;
    //for(ld_tmp_ptr = 0; ld_tmp_ptr < L2PE_QNTY; ld_tmp_ptr = ld_tmp_ptr + 1)
    //{
    //    gd_l2p_tbl[ld_tmp_ptr] = L2P_NULL;
    //}
    // memset((byte *)(FTL_L2P_BASE), 0x0, L2PE_QNTY*4);
    dwrd lq_l2pad_qty = ((dwrd)L2PE_QNTY)*4;
    fv_uart_print("clearing l2p table,size %lu(MB) ,may cost some seconds.\r\n", lq_l2pad_qty/1000000);
    if(lq_l2pad_qty >= 0x80000000){
		rb_cdir_bas = 0;
		memset((byte *)(FTL_L2P_BASE), 0x0, 0x40000000);
		memset((byte *)(FTL_L2P_BASE + 0x40000000), 0x0, 0x40000000);
		rb_cdir_bas = 1;
		if(lq_l2pad_qty != 0x80000000){
			memset((byte *)(FTL_L2P_BASE), 0x0, (lq_l2pad_qty & 0x7fffffff));
		}
    }else{
		memset((byte *)(FTL_L2P_BASE), 0x0, lq_l2pad_qty);
		rb_cdir_bas = 1;
    }

  #ifdef FTL_DBG
    //compression location table init
    memset((byte *)(FTL_CRL_BASE), 0x0, L2PE_QNTY);

    //valid bitmap table init
    for(lw_blk_ptr = 0; lw_blk_ptr < RBLK_QNTY; lw_blk_ptr = lw_blk_ptr + 1)
    {
        memset((byte *)(&gd_vmap_tbl[((dwrd)lw_blk_ptr << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT + CMPR_SHIFT - 5))]), 0x0, (1 << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT + CMPR_SHIFT - 3)));
    }
  #endif
#endif

    ARM_NOP();
    ARM_NOP();

    //Valid Cnt Table init
    for(lw_blk_ptr = 0; lw_blk_ptr < RBLK_QNTY; lw_blk_ptr = lw_blk_ptr + 1)
    {
        gd_vcnt_tbl[lw_blk_ptr] = 0x0;
    }
    //memset((byte *)(&gd_vcnt_tbl[0]), 0x0, RBLK_QNTY*4);

    //Erase Cnt Table init
    for(lw_blk_ptr = 0; lw_blk_ptr < RBLK_QNTY; lw_blk_ptr = lw_blk_ptr + 1)
    {
        gd_ecnt_tbl[lw_blk_ptr] = 0x0;
    }
    //memset((byte *)(&gd_ecnt_tbl[0]), 0x0, RBLK_QNTY*4);


    //erase build table
    gw_fblk_str = DBLK_INVLD;
    gw_bblk_str = DBLK_INVLD;

    for(lw_blk_ptr = 1; lw_blk_ptr < RBLK_QNTY; lw_blk_ptr = lw_blk_ptr + 1)
    {
        lb_good_blk = 1;

        for(lb_ce = 0; lb_ce < BANK_QNTY; lb_ce = lb_ce + 1)
        {
            for(lb_chan = 0; lb_chan < CHAN_QNTY; lb_chan = lb_chan + 1)
            {
                lb_cech = (lb_ce << CHAN_SHIFT) | lb_chan;
                rd_cmd_padr = ((dwrd)lw_blk_ptr << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) | ((dwrd)lb_cech << FRAG_SHIFT);
                fv_nque_chk(20);
                rw_cmd_type = ERS_CPTR | LOW_PRI | CMD_END;
            }
        }

        for(lb_ce = 0; lb_ce < BANK_QNTY; lb_ce = lb_ce + 1)
        {
            for(lb_chan = 0; lb_chan < CHAN_QNTY; lb_chan = lb_chan + 1)
            {
                //check erase status
                rb_index = lb_chan;
                while(rb_cmd_sts != ALL_FIN);
                while(rd_cmdq_vld != ALL_INVLD);
                while(rd_que_rdy != ALL_READY);
                while(rb_qabt_fsm != IDLE_ST);
                while((rw_sts_rdy & (1 << lb_ce)) == 0);

                if(rw_sts_fail & (1 << lb_ce)) //bad block
                {
					fv_uart_print("bad block: ce:%x, chan:%x, blk_ptr:%x\r\n", lb_ce, lb_chan, lw_blk_ptr);

                    //change H/W, write 1 clear
                    rw_sts_fail = rw_sts_fail & (1 << lb_ce); //clear erase fail status
                    lb_good_blk = 0;
                }

                if((lb_ce == (BANK_QNTY - 1)) && (lb_chan == (CHAN_QNTY - 1)))
                {
                    if(lb_good_blk == 0)
                    {
                        if(gw_bblk_str == DBLK_INVLD)
                            gw_bblk_str = lw_blk_ptr;
                        else
                            gs_dbl_tbl[gw_bblk_end].wd.w_next_ptr = lw_blk_ptr;

                        gw_bblk_end = lw_blk_ptr;
                        gs_dbl_tbl[lw_blk_ptr].wd.w_next_ptr = DBLK_INVLD;
                        gb_blk_type[lw_blk_ptr] = BAD_BLK;
                    }
                    else
                    {
                        gw_fblk_num = gw_fblk_num + 0x1;

                        if(gw_fblk_str == DBLK_INVLD)
                            gw_fblk_str = lw_blk_ptr;
                        else
                            gs_dbl_tbl[gw_fblk_end].wd.w_next_ptr = lw_blk_ptr;

                        gw_fblk_end = lw_blk_ptr;
                        gb_blk_type[lw_blk_ptr] = FREE_BLK;

                    }
                }
            }
        }


        if(gw_fblk_num == VBLK_QNTY)
        {
            fv_uart_print("free blk num reach the VBLK_QNTY\r\n");

            lw_blk_ptr = lw_blk_ptr + 1;
            while(lw_blk_ptr < RBLK_QNTY)
            {
                if(gw_bblk_str == DBLK_INVLD)
                    gw_bblk_str = lw_blk_ptr;
                else
                    gs_dbl_tbl[gw_bblk_end].wd.w_next_ptr = lw_blk_ptr;

                gw_bblk_end = lw_blk_ptr;
                gs_dbl_tbl[lw_blk_ptr].wd.w_next_ptr = DBLK_INVLD;
                lw_blk_ptr = lw_blk_ptr + 1;
                gb_blk_type[lw_blk_ptr] = BAD_BLK;
            }

            break;
        }
    }

    fv_uart_print("free blk num:%d, VBLK_QNTY: %d\r\n", gw_fblk_num, VBLK_QNTY);
    return;
}


void fv_gctbl_init(void)
{
    byte lb_head_wcnt = 0x0;
#if defined(NAND_BICS5) || defined(NAND_YX39070)
    byte lb_head_wend = 0x2;
#else
    byte lb_head_wend = 0x0;
#endif
    byte lb_head_frag = 0x0;
    byte lb_head_cech = 0x0;
    word lw_head_page = 0x0;
    word lw_mapu_cnt = 0;
    word lw_pmsg_ptr = 0;
    word lw_next_page;
    word lw_end_page;
    word lw_min_page;
    word lw_max_page;
    word lw_tmp_page;

    //calculate gc table
    lw_next_page = 0;
    lw_end_page = (1 << (15 - BKCH_SHIFT - FRAG_SHIFT));
    lw_min_page = 0;
    lw_max_page = 0;
    lw_tmp_page = 0;
    while(1)
    {
        if(lb_head_frag != (FRAG_QNTY - 1))
        {
            lb_head_frag++;
        }
        else
        {
            lb_head_frag = 0x0;
            if(lb_head_wcnt != lb_head_wend)
            {
                lw_head_page++;
                lb_head_wcnt++;
                if(lw_head_page > lw_tmp_page)
                {
                    lw_tmp_page = lw_head_page;
                }
            }
            else
            {
                lb_head_wcnt = 0x0;
                if(lb_head_cech != (RDIE_QNTY - 1))
                {
                    lb_head_cech++;
                    lw_head_page = lw_head_page - (word)lb_head_wend;
                }
                else
                {
                    lb_head_cech = 0x0;
                    if(lw_head_page != (PAGE_QNTY - 1))
                    {
                        lw_head_page++;
                    }
                    else
                    {
                        if(lw_pmsg_ptr != PMSG_END)
                        {
                            fv_uart_print("p2l page location error\r\n");
                            fv_dbg_loop(0x11);
                        }
                        gd_pmsg_pcf[lw_pmsg_ptr] = ((((PAGE_QNTY - 1) << BKCH_SHIFT) | (RDIE_QNTY - 1)) << FRAG_SHIFT) | (FRAG_QNTY - 4);
                        gb_pmsg_npg[lw_pmsg_ptr] = 0;
                        gb_pmsg_epg[lw_pmsg_ptr] = 1;

                        break;
                    }

#if (!defined(SLC_MODE) && !(defined(NAND_BICS5) || defined(NAND_YX39070)))
                    if((lw_head_page < 4) || (lw_head_page >= 2108))
                        lb_head_wend = 0x0;
                    else if((lw_head_page >= 1048) && (lw_head_page < 1064))
                        lb_head_wend = 0x1;
                    else
                        lb_head_wend = 0x2;
#endif

                    lw_min_page = lw_head_page;
                    lw_tmp_page = lw_head_page;
                }
            }
        }

        lw_mapu_cnt = (lw_mapu_cnt < 4096) ? (lw_mapu_cnt + CMPR_NUM) : (lw_mapu_cnt + 1);
        //fv_uart_print("%d, %x, %x, %d\r\n", lw_mapu_cnt, lb_head_frag, lb_head_cech, lw_head_page);
        if(lw_mapu_cnt == 4096)
        {
            gd_pmsg_pcf[lw_pmsg_ptr] = ((((dwrd)lw_head_page << BKCH_SHIFT) | (dwrd)lb_head_cech) << FRAG_SHIFT) | (dwrd)lb_head_frag;

            if(lw_max_page < lw_next_page)
            {
                gb_pmsg_npg[lw_pmsg_ptr] = 0;
            }
            else
            {
                gb_pmsg_npg[lw_pmsg_ptr] = 1;
                lw_next_page = lw_next_page + (1 << (15 - BKCH_SHIFT - FRAG_SHIFT));
            }
        }
        else if(lw_mapu_cnt == (4096 + 4))
        {
            lw_mapu_cnt = 0x0;

	    if(lw_min_page < lw_end_page)
            {
                gb_pmsg_epg[lw_pmsg_ptr] = 0;
            }
            else
            {
                gb_pmsg_epg[lw_pmsg_ptr] = 1;
                lw_end_page = lw_end_page + (1 << (15 - BKCH_SHIFT - FRAG_SHIFT));
            }

            lw_pmsg_ptr++;
        }

        lw_max_page = lw_tmp_page;

#ifdef RTL_SIM
        if(lw_pmsg_ptr == 30)
        {
            break;
        }
#endif
    } //while(1)

    return;
} 

void fv_ftl_pre(void)
{
    byte lb_cnt;

    gd_l2p_tbl = (dwrd *)(FTL_L2P_BASE);
    gd_p2l_tbl = (dwrd *)(FTL_P2L_BASE);
    gd_vmap_tbl = (dwrd *)(FTL_VMAP_BASE);
    gb_crl_tbl = (byte *)(FTL_CRL_BASE);

    gb_get_htblk = 1;
    gb_get_gcblk = 1;

    for(lb_cnt = 0; lb_cnt < 2; lb_cnt = lb_cnt + 1)
    {
        gb_p2lp_nrd[lb_cnt] = 0;
        gb_p2lp_cnt[lb_cnt] = 0;
        gb_gcad_wat[lb_cnt] = 0;
    }

    for(lb_cnt = 0; lb_cnt < 8; lb_cnt = lb_cnt + 1)
    {
        gw_chkz_cnt[lb_cnt] = 0;
        gb_chkz_fin[lb_cnt] = 0;
    }

    for(lb_cnt = 0; lb_cnt < HEAD_QNTY; lb_cnt = lb_cnt + 1)
    {
        gs_head_padr[lb_cnt].sb_head_wcnt = 0x0;
#if defined(NAND_BICS5) || defined(NAND_YX39070)
        gs_head_padr[lb_cnt].sb_head_wend = 0x2;
#else
        gs_head_padr[lb_cnt].sb_head_wend = 0x0;
#endif
        gs_head_padr[lb_cnt].sb_head_frag = 0x0;
        gs_head_padr[lb_cnt].sb_head_cech = 0x0;
        gs_head_padr[lb_cnt].sw_head_page = 0x0;
        //gs_head_padr[lb_cnt].sw_head_blk  = 0x0;
    }

    //initial dram base address
    rw_dmdt_bas = (DRAM_DATA_BASE  >> (12 + 12));
    rw_dmmt_bas = (DRAM_META_BASE >> (12 + 8));

    //initial nptr table
    rb_itbl_set = ITBL_ACT;
    while(rb_itbl_set & ITBL_ACT);

#ifdef LGET_EN
    rb_lget_set = LGET_ACT;  //enable HW auto get l2p function
#endif

#ifdef LUPD_EN
    rd_fcfg_set |= GCWR_ADD; //enable HW update "gc write info" into "BMU buffer table"
    rd_fcfg_set |= (1 << 15);
    rb_hacc_fun |= BSTS_CHK; //gc write would check buffer status
#endif

#ifdef CMPR_EN
    rb_cmpr_ena = 1;         //enable compression function
    rd_fcfg_set |= RMAP_HID; //remap hid for compression
    rd_fcfg_set |= HFRC_ADD; //force host write cmd add into bmu(not overwrite)
#endif

#ifdef HWCMD_CHK
    rd_fcfg_set |= CACQ_ALL; //check the bmu handle cmd
#endif

#ifdef DIE_RAID
#ifdef DIE_RAID5_EN
    rd_fcfg_set |= RAID5_ENA; //enable raid5 function
#else
    rd_fcfg_set |= RAID_ENA; //enable raid6 function
#endif
    rd_rgrp_cfg = FRAG_SHIFT | (BKCH_SHIFT << 8);
    rb_rtbl_num = BKCH_QNTY - RAID_PNUM;
#endif

    //clear overwrite bimap
    rb_oclr_map = 0xff;
    while(rb_oclr_map);

    //get free sram buffer ptr
    for(lb_cnt = 0; lb_cnt < 12; lb_cnt = lb_cnt + 1)
    {
        rb_cafp_set = CAFP_ACT | SRAM_BUF | CPUA_GET;
        while(rb_cafp_set & CAFP_ACT);
        if(rb_cafp_rlt == 0)
        {
            fv_uart_print("free buffer error\r\n");
            fv_dbg_loop(0x05);
        }
    }

    //buffer allocate threshold
#ifdef DIS_DRAM_BUF
    rw_htsl_thr = 8;
    rw_htsb_thr = 60;
    rw_gcdl_thr = 0;
    rw_gcdb_thr = 0;
    rw_gcsl_thr = 8;
    rw_gcsb_thr = 160;
#else
    rw_gcdl_thr = 64;
    rw_gcdb_thr = 2400;
	rw_tbsb_thr = 32;
    rw_tbpb_thr = 32;
    rw_ndpb_thr = 240;
#endif

    //set buffer ready
    rb_dbuf_rdy = 1;
    rb_sbuf_rdy = 1;
    rb_pbuf_rdy = 1;

    //gc table initial
#if (defined(RTL_SIM) && (defined(FPGA_MODE) || defined(FPGA_FAST_SIM))) //force BiCS5/8channel/1CE
  #if (defined(DIE_RAID) && defined(CMPR_EN))
    gd_pmsg_pcf[ 0] = 0x000005c0;
    gd_pmsg_pcf[ 1] = 0x00000acc;
    gd_pmsg_pcf[ 2] = 0x00001010;
    gd_pmsg_pcf[ 3] = 0x0000151c;
    gd_pmsg_pcf[ 4] = 0x00001a60;
    gd_pmsg_pcf[ 5] = 0x00002024;
    gd_pmsg_pcf[ 6] = 0x00002568;
    gd_pmsg_pcf[ 7] = 0x00002b04;
    gd_pmsg_pcf[ 8] = 0x00003048;
    gd_pmsg_pcf[ 9] = 0x00003554;
    gd_pmsg_pcf[10] = 0x00003a98;
    gd_pmsg_pcf[11] = 0x0000405c;
    gd_pmsg_pcf[12] = 0x000045a0;
    gd_pmsg_pcf[13] = 0x00004aac;
    gd_pmsg_pcf[14] = 0x00005080;
    gd_pmsg_pcf[15] = 0x0000558c;
    gd_pmsg_pcf[16] = 0x00005ad0;
    gd_pmsg_pcf[17] = 0x00006094;
    gd_pmsg_pcf[18] = 0x000065d8;
    gd_pmsg_pcf[19] = 0x00006ae4;
    gd_pmsg_pcf[20] = 0x00007028;
    gd_pmsg_pcf[21] = 0x000075c4;
    gd_pmsg_pcf[22] = 0x00007b08;
    gd_pmsg_pcf[23] = 0x000080cc;
    gd_pmsg_pcf[24] = 0x00008610;
    gd_pmsg_pcf[25] = 0x00008b1c;
    gd_pmsg_pcf[26] = 0x00009060;
    gd_pmsg_pcf[27] = 0x0000956c;
    gd_pmsg_pcf[28] = 0x00009b40;
    gd_pmsg_pcf[29] = 0x0000a104;
    gd_pmsg_pcf[30] = 0x0000a648;
    gd_pmsg_pcf[31] = 0x0000ab54;
    gd_pmsg_pcf[32] = 0x0000b098;
    gd_pmsg_pcf[33] = 0x0000b5a4;
    gd_pmsg_pcf[34] = 0x0000bae8;
    gd_pmsg_pcf[35] = 0x0000c0ac;
    gd_pmsg_pcf[36] = 0x0000c680;
    gd_pmsg_pcf[37] = 0x0000cb8c;
    gd_pmsg_pcf[38] = 0x0000d0d0;
    gd_pmsg_pcf[39] = 0x0000d5dc;
    gd_pmsg_pcf[40] = 0x0000db20;
    gd_pmsg_pcf[41] = 0x0000e0e4;
    gd_pmsg_pcf[42] = 0x0000e628;
    gd_pmsg_pcf[43] = 0x0000ebc4;
    gd_pmsg_pcf[44] = 0x0000f108;
    gd_pmsg_pcf[45] = 0x0000f614;
    gd_pmsg_pcf[46] = 0x0000fb58;
    gd_pmsg_pcf[47] = 0x0001011c;
    gd_pmsg_pcf[48] = 0x00010660;
    gd_pmsg_pcf[49] = 0x00010b6c;
    gd_pmsg_pcf[50] = 0x00011140;
    gd_pmsg_pcf[51] = 0x0001164c;
    gd_pmsg_pcf[52] = 0x00011b90;
    gd_pmsg_pcf[53] = 0x00011fec;
    gb_pmsg_npg[ 0] = 1;
    gb_pmsg_npg[ 1] = 0;
    gb_pmsg_npg[ 2] = 0;
    gb_pmsg_npg[ 3] = 0;
    gb_pmsg_npg[ 4] = 0;
    gb_pmsg_npg[ 5] = 0;
    gb_pmsg_npg[ 6] = 0;
    gb_pmsg_npg[ 7] = 0;
    gb_pmsg_npg[ 8] = 0;
    gb_pmsg_npg[ 9] = 0;
    gb_pmsg_npg[10] = 0;
    gb_pmsg_npg[11] = 0;
    gb_pmsg_npg[12] = 0;
    gb_pmsg_npg[13] = 0;
    gb_pmsg_npg[14] = 0;
    gb_pmsg_npg[15] = 0;
    gb_pmsg_npg[16] = 0;
    gb_pmsg_npg[17] = 0;
    gb_pmsg_npg[18] = 0;
    gb_pmsg_npg[19] = 0;
    gb_pmsg_npg[20] = 0;
    gb_pmsg_npg[21] = 0;
    gb_pmsg_npg[22] = 0;
    gb_pmsg_npg[23] = 1;
    gb_pmsg_npg[24] = 0;
    gb_pmsg_npg[25] = 0;
    gb_pmsg_npg[26] = 0;
    gb_pmsg_npg[27] = 0;
    gb_pmsg_npg[28] = 0;
    gb_pmsg_npg[29] = 0;
    gb_pmsg_npg[30] = 0;
    gb_pmsg_npg[31] = 0;
    gb_pmsg_npg[32] = 0;
    gb_pmsg_npg[33] = 0;
    gb_pmsg_npg[34] = 0;
    gb_pmsg_npg[35] = 0;
    gb_pmsg_npg[36] = 0;
    gb_pmsg_npg[37] = 0;
    gb_pmsg_npg[38] = 0;
    gb_pmsg_npg[39] = 0;
    gb_pmsg_npg[40] = 0;
    gb_pmsg_npg[41] = 0;
    gb_pmsg_npg[42] = 0;
    gb_pmsg_npg[43] = 0;
    gb_pmsg_npg[44] = 0;
    gb_pmsg_npg[45] = 0;
    gb_pmsg_npg[46] = 0;
    gb_pmsg_npg[47] = 1;
    gb_pmsg_npg[48] = 0;
    gb_pmsg_npg[49] = 0;
    gb_pmsg_npg[50] = 0;
    gb_pmsg_npg[51] = 0;
    gb_pmsg_npg[52] = 0;
    gb_pmsg_npg[53] = 0;
    gb_pmsg_epg[ 0] = 0;
    gb_pmsg_epg[ 1] = 0;
    gb_pmsg_epg[ 2] = 0;
    gb_pmsg_epg[ 3] = 0;
    gb_pmsg_epg[ 4] = 0;
    gb_pmsg_epg[ 5] = 0;
    gb_pmsg_epg[ 6] = 0;
    gb_pmsg_epg[ 7] = 0;
    gb_pmsg_epg[ 8] = 0;
    gb_pmsg_epg[ 9] = 0;
    gb_pmsg_epg[10] = 0;
    gb_pmsg_epg[11] = 0;
    gb_pmsg_epg[12] = 0;
    gb_pmsg_epg[13] = 0;
    gb_pmsg_epg[14] = 0;
    gb_pmsg_epg[15] = 0;
    gb_pmsg_epg[16] = 0;
    gb_pmsg_epg[17] = 0;
    gb_pmsg_epg[18] = 0;
    gb_pmsg_epg[19] = 0;
    gb_pmsg_epg[20] = 0;
    gb_pmsg_epg[21] = 0;
    gb_pmsg_epg[22] = 0;
    gb_pmsg_epg[23] = 1;
    gb_pmsg_epg[24] = 0;
    gb_pmsg_epg[25] = 0;
    gb_pmsg_epg[26] = 0;
    gb_pmsg_epg[27] = 0;
    gb_pmsg_epg[28] = 0;
    gb_pmsg_epg[29] = 0;
    gb_pmsg_epg[30] = 0;
    gb_pmsg_epg[31] = 0;
    gb_pmsg_epg[32] = 0;
    gb_pmsg_epg[33] = 0;
    gb_pmsg_epg[34] = 0;
    gb_pmsg_epg[35] = 0;
    gb_pmsg_epg[36] = 0;
    gb_pmsg_epg[37] = 0;
    gb_pmsg_epg[38] = 0;
    gb_pmsg_epg[39] = 0;
    gb_pmsg_epg[40] = 0;
    gb_pmsg_epg[41] = 0;
    gb_pmsg_epg[42] = 0;
    gb_pmsg_epg[43] = 0;
    gb_pmsg_epg[44] = 0;
    gb_pmsg_epg[45] = 0;
    gb_pmsg_epg[46] = 0;
    gb_pmsg_epg[47] = 1;
    gb_pmsg_epg[48] = 0;
    gb_pmsg_epg[49] = 0;
    gb_pmsg_epg[50] = 0;
    gb_pmsg_epg[51] = 0;
    gb_pmsg_epg[52] = 0;
    gb_pmsg_epg[53] = 1;

  #elif (defined(DIE_RAID) && !defined(CMPR_EN))
    gd_pmsg_pcf[ 0] = 0x00001590;
    gd_pmsg_pcf[ 1] = 0x00002a6c;
    gd_pmsg_pcf[ 2] = 0x00004010;
    gd_pmsg_pcf[ 3] = 0x000054ec;
    gd_pmsg_pcf[ 4] = 0x00006a90;
    gd_pmsg_pcf[ 5] = 0x00008024;
    gd_pmsg_pcf[ 6] = 0x000095c8;
    gd_pmsg_pcf[ 7] = 0x0000aaa4;
    gd_pmsg_pcf[ 8] = 0x0000c048;
    gd_pmsg_pcf[ 9] = 0x0000d524;
    gd_pmsg_pcf[10] = 0x0000eac8;
    gd_pmsg_pcf[11] = 0x0001005c;
    gd_pmsg_pcf[12] = 0x00011600;
    gd_pmsg_pcf[13] = 0x00011fec;
    gb_pmsg_npg[ 0] = 1;
    gb_pmsg_npg[ 1] = 0;
    gb_pmsg_npg[ 2] = 0;
    gb_pmsg_npg[ 3] = 0;
    gb_pmsg_npg[ 4] = 0;
    gb_pmsg_npg[ 5] = 1;
    gb_pmsg_npg[ 6] = 0;
    gb_pmsg_npg[ 7] = 0;
    gb_pmsg_npg[ 8] = 0;
    gb_pmsg_npg[ 9] = 0;
    gb_pmsg_npg[10] = 0;
    gb_pmsg_npg[11] = 1;
    gb_pmsg_npg[12] = 0;
    gb_pmsg_npg[13] = 0;
    gb_pmsg_epg[ 0] = 0;
    gb_pmsg_epg[ 1] = 0;
    gb_pmsg_epg[ 2] = 0;
    gb_pmsg_epg[ 3] = 0;
    gb_pmsg_epg[ 4] = 0;
    gb_pmsg_epg[ 5] = 0;
    gb_pmsg_epg[ 6] = 1;
    gb_pmsg_epg[ 7] = 0;
    gb_pmsg_epg[ 8] = 0;
    gb_pmsg_epg[ 9] = 0;
    gb_pmsg_epg[10] = 0;
    gb_pmsg_epg[11] = 0;
    gb_pmsg_epg[12] = 1;
    gb_pmsg_epg[13] = 1;

  #elif (!defined(DIE_RAID) && defined(CMPR_EN))
    gd_pmsg_pcf[ 0] = 0x00000450;
    gd_pmsg_pcf[ 1] = 0x000007ec;
    gd_pmsg_pcf[ 2] = 0x00000c40;
    gd_pmsg_pcf[ 3] = 0x00000fdc;
    gd_pmsg_pcf[ 4] = 0x000013b0;
    gd_pmsg_pcf[ 5] = 0x00001884;
    gd_pmsg_pcf[ 6] = 0x00001c58;
    gd_pmsg_pcf[ 7] = 0x00001ff4;
    gd_pmsg_pcf[ 8] = 0x00002448;
    gd_pmsg_pcf[ 9] = 0x000027e4;
    gd_pmsg_pcf[10] = 0x00002bb8;
    gd_pmsg_pcf[11] = 0x0000308c;
    gd_pmsg_pcf[12] = 0x00003460;
    gd_pmsg_pcf[13] = 0x000037fc;
    gd_pmsg_pcf[14] = 0x00003c50;
    gd_pmsg_pcf[15] = 0x00003fec;
    gd_pmsg_pcf[16] = 0x00004440;
    gd_pmsg_pcf[17] = 0x00004894;
    gd_pmsg_pcf[18] = 0x00004c68;
    gd_pmsg_pcf[19] = 0x00005084;
    gd_pmsg_pcf[20] = 0x00005458;
    gd_pmsg_pcf[21] = 0x000057f4;
    gd_pmsg_pcf[22] = 0x00005c48;
    gd_pmsg_pcf[23] = 0x0000609c;
    gd_pmsg_pcf[24] = 0x00006470;
    gd_pmsg_pcf[25] = 0x0000688c;
    gd_pmsg_pcf[26] = 0x00006c60;
    gd_pmsg_pcf[27] = 0x00006ffc;
    gd_pmsg_pcf[28] = 0x00007450;
    gd_pmsg_pcf[29] = 0x000078a4;
    gd_pmsg_pcf[30] = 0x00007c78;
    gd_pmsg_pcf[31] = 0x00008094;
    gd_pmsg_pcf[32] = 0x00008468;
    gd_pmsg_pcf[33] = 0x00008884;
    gd_pmsg_pcf[34] = 0x00008c58;
    gd_pmsg_pcf[35] = 0x000090ac;
    gd_pmsg_pcf[36] = 0x00009500;
    gd_pmsg_pcf[37] = 0x0000989c;
    gd_pmsg_pcf[38] = 0x00009c70;
    gd_pmsg_pcf[39] = 0x0000a08c;
    gd_pmsg_pcf[40] = 0x0000a460;
    gd_pmsg_pcf[41] = 0x0000a8b4;
    gd_pmsg_pcf[42] = 0x0000ad08;
    gd_pmsg_pcf[43] = 0x0000b0a4;
    gd_pmsg_pcf[44] = 0x0000b478;
    gd_pmsg_pcf[45] = 0x0000b894;
    gd_pmsg_pcf[46] = 0x0000bc68;
    gd_pmsg_pcf[47] = 0x0000c0bc;
    gd_pmsg_pcf[48] = 0x0000c510;
    gd_pmsg_pcf[49] = 0x0000c8ac;
    gd_pmsg_pcf[50] = 0x0000cd00;
    gd_pmsg_pcf[51] = 0x0000d09c;
    gd_pmsg_pcf[52] = 0x0000d470;
    gd_pmsg_pcf[53] = 0x0000d944;
    gd_pmsg_pcf[54] = 0x0000dd18;
    gd_pmsg_pcf[55] = 0x0000e0b4;
    gd_pmsg_pcf[56] = 0x0000e508;
    gd_pmsg_pcf[57] = 0x0000e8a4;
    gd_pmsg_pcf[58] = 0x0000ec78;
    gd_pmsg_pcf[59] = 0x0000f14c;
    gd_pmsg_pcf[60] = 0x0000f520;
    gd_pmsg_pcf[61] = 0x0000f8bc;
    gd_pmsg_pcf[62] = 0x0000fd10;
    gd_pmsg_pcf[63] = 0x000100ac;
    gd_pmsg_pcf[64] = 0x00010500;
    gd_pmsg_pcf[65] = 0x00010954;
    gd_pmsg_pcf[66] = 0x00010d28;
    gd_pmsg_pcf[67] = 0x00011144;
    gd_pmsg_pcf[68] = 0x00011518;
    gd_pmsg_pcf[69] = 0x000118b4;
    gd_pmsg_pcf[70] = 0x00011d08;
    gd_pmsg_pcf[71] = 0x00011ffc;
    gb_pmsg_npg[ 0] = 1;
    gb_pmsg_npg[ 1] = 0;
    gb_pmsg_npg[ 2] = 0;
    gb_pmsg_npg[ 3] = 0;
    gb_pmsg_npg[ 4] = 0;
    gb_pmsg_npg[ 5] = 0;
    gb_pmsg_npg[ 6] = 0;
    gb_pmsg_npg[ 7] = 0;
    gb_pmsg_npg[ 8] = 0;
    gb_pmsg_npg[ 9] = 0;
    gb_pmsg_npg[10] = 0;
    gb_pmsg_npg[11] = 0;
    gb_pmsg_npg[12] = 0;
    gb_pmsg_npg[13] = 0;
    gb_pmsg_npg[14] = 0;
    gb_pmsg_npg[15] = 0;
    gb_pmsg_npg[16] = 0;
    gb_pmsg_npg[17] = 0;
    gb_pmsg_npg[18] = 0;
    gb_pmsg_npg[19] = 0;
    gb_pmsg_npg[20] = 0;
    gb_pmsg_npg[21] = 0;
    gb_pmsg_npg[22] = 0;
    gb_pmsg_npg[23] = 0;
    gb_pmsg_npg[24] = 0;
    gb_pmsg_npg[25] = 0;
    gb_pmsg_npg[26] = 0;
    gb_pmsg_npg[27] = 0;
    gb_pmsg_npg[28] = 0;
    gb_pmsg_npg[29] = 0;
    gb_pmsg_npg[30] = 0;
    gb_pmsg_npg[31] = 1;
    gb_pmsg_npg[32] = 0;
    gb_pmsg_npg[33] = 0;
    gb_pmsg_npg[34] = 0;
    gb_pmsg_npg[35] = 0;
    gb_pmsg_npg[36] = 0;
    gb_pmsg_npg[37] = 0;
    gb_pmsg_npg[38] = 0;
    gb_pmsg_npg[39] = 0;
    gb_pmsg_npg[40] = 0;
    gb_pmsg_npg[41] = 0;
    gb_pmsg_npg[42] = 0;
    gb_pmsg_npg[43] = 0;
    gb_pmsg_npg[44] = 0;
    gb_pmsg_npg[45] = 0;
    gb_pmsg_npg[46] = 0;
    gb_pmsg_npg[47] = 0;
    gb_pmsg_npg[48] = 0;
    gb_pmsg_npg[49] = 0;
    gb_pmsg_npg[50] = 0;
    gb_pmsg_npg[51] = 0;
    gb_pmsg_npg[52] = 0;
    gb_pmsg_npg[53] = 0;
    gb_pmsg_npg[54] = 0;
    gb_pmsg_npg[55] = 0;
    gb_pmsg_npg[56] = 0;
    gb_pmsg_npg[57] = 0;
    gb_pmsg_npg[58] = 0;
    gb_pmsg_npg[59] = 0;
    gb_pmsg_npg[60] = 0;
    gb_pmsg_npg[61] = 0;
    gb_pmsg_npg[62] = 0;
    gb_pmsg_npg[63] = 1;
    gb_pmsg_npg[64] = 0;
    gb_pmsg_npg[65] = 0;
    gb_pmsg_npg[66] = 0;
    gb_pmsg_npg[67] = 0;
    gb_pmsg_npg[68] = 0;
    gb_pmsg_npg[69] = 0;
    gb_pmsg_npg[70] = 0;
    gb_pmsg_npg[71] = 0;
    gb_pmsg_epg[ 0] = 0;
    gb_pmsg_epg[ 1] = 0;
    gb_pmsg_epg[ 2] = 0;
    gb_pmsg_epg[ 3] = 0;
    gb_pmsg_epg[ 4] = 0;
    gb_pmsg_epg[ 5] = 0;
    gb_pmsg_epg[ 6] = 0;
    gb_pmsg_epg[ 7] = 0;
    gb_pmsg_epg[ 8] = 0;
    gb_pmsg_epg[ 9] = 0;
    gb_pmsg_epg[10] = 0;
    gb_pmsg_epg[11] = 0;
    gb_pmsg_epg[12] = 0;
    gb_pmsg_epg[13] = 0;
    gb_pmsg_epg[14] = 0;
    gb_pmsg_epg[15] = 0;
    gb_pmsg_epg[16] = 0;
    gb_pmsg_epg[17] = 0;
    gb_pmsg_epg[18] = 0;
    gb_pmsg_epg[19] = 0;
    gb_pmsg_epg[20] = 0;
    gb_pmsg_epg[21] = 0;
    gb_pmsg_epg[22] = 0;
    gb_pmsg_epg[23] = 0;
    gb_pmsg_epg[24] = 0;
    gb_pmsg_epg[25] = 0;
    gb_pmsg_epg[26] = 0;
    gb_pmsg_epg[27] = 0;
    gb_pmsg_epg[28] = 0;
    gb_pmsg_epg[29] = 0;
    gb_pmsg_epg[30] = 0;
    gb_pmsg_epg[31] = 1;
    gb_pmsg_epg[32] = 0;
    gb_pmsg_epg[33] = 0;
    gb_pmsg_epg[34] = 0;
    gb_pmsg_epg[35] = 0;
    gb_pmsg_epg[36] = 0;
    gb_pmsg_epg[37] = 0;
    gb_pmsg_epg[38] = 0;
    gb_pmsg_epg[39] = 0;
    gb_pmsg_epg[40] = 0;
    gb_pmsg_epg[41] = 0;
    gb_pmsg_epg[42] = 0;
    gb_pmsg_epg[43] = 0;
    gb_pmsg_epg[44] = 0;
    gb_pmsg_epg[45] = 0;
    gb_pmsg_epg[46] = 0;
    gb_pmsg_epg[47] = 0;
    gb_pmsg_epg[48] = 0;
    gb_pmsg_epg[49] = 0;
    gb_pmsg_epg[50] = 0;
    gb_pmsg_epg[51] = 0;
    gb_pmsg_epg[52] = 0;
    gb_pmsg_epg[53] = 0;
    gb_pmsg_epg[54] = 0;
    gb_pmsg_epg[55] = 0;
    gb_pmsg_epg[56] = 0;
    gb_pmsg_epg[57] = 0;
    gb_pmsg_epg[58] = 0;
    gb_pmsg_epg[59] = 0;
    gb_pmsg_epg[60] = 0;
    gb_pmsg_epg[61] = 0;
    gb_pmsg_epg[62] = 0;
    gb_pmsg_epg[63] = 1;
    gb_pmsg_epg[64] = 0;
    gb_pmsg_epg[65] = 0;
    gb_pmsg_epg[66] = 0;
    gb_pmsg_epg[67] = 0;
    gb_pmsg_epg[68] = 0;
    gb_pmsg_epg[69] = 0;
    gb_pmsg_epg[70] = 0;
    gb_pmsg_epg[71] = 1;

  #else
    gd_pmsg_pcf[ 0] = 0x00001050;
    gd_pmsg_pcf[ 1] = 0x00001fec;
    gd_pmsg_pcf[ 2] = 0x00003040;
    gd_pmsg_pcf[ 3] = 0x00003fdc;
    gd_pmsg_pcf[ 4] = 0x00004fb0;
    gd_pmsg_pcf[ 5] = 0x00006084;
    gd_pmsg_pcf[ 6] = 0x00007058;
    gd_pmsg_pcf[ 7] = 0x00007ff4;
    gd_pmsg_pcf[ 8] = 0x00009048;
    gd_pmsg_pcf[ 9] = 0x00009fe4;
    gd_pmsg_pcf[10] = 0x0000afb8;
    gd_pmsg_pcf[11] = 0x0000c08c;
    gd_pmsg_pcf[12] = 0x0000d060;
    gd_pmsg_pcf[13] = 0x0000dffc;
    gd_pmsg_pcf[14] = 0x0000f050;
    gd_pmsg_pcf[15] = 0x0000ffec;
    gd_pmsg_pcf[16] = 0x00011040;
    gd_pmsg_pcf[17] = 0x00011ffc;
    gb_pmsg_npg[ 0] = 1;
    gb_pmsg_npg[ 1] = 0;
    gb_pmsg_npg[ 2] = 0;
    gb_pmsg_npg[ 3] = 0;
    gb_pmsg_npg[ 4] = 0;
    gb_pmsg_npg[ 5] = 0;
    gb_pmsg_npg[ 6] = 0;
    gb_pmsg_npg[ 7] = 1;
    gb_pmsg_npg[ 8] = 0;
    gb_pmsg_npg[ 9] = 0;
    gb_pmsg_npg[10] = 0;
    gb_pmsg_npg[11] = 0;
    gb_pmsg_npg[12] = 0;
    gb_pmsg_npg[13] = 0;
    gb_pmsg_npg[14] = 0;
    gb_pmsg_npg[15] = 1;
    gb_pmsg_npg[16] = 0;
    gb_pmsg_npg[17] = 0;
    gb_pmsg_epg[ 0] = 0;
    gb_pmsg_epg[ 1] = 0;
    gb_pmsg_epg[ 2] = 0;
    gb_pmsg_epg[ 3] = 0;
    gb_pmsg_epg[ 4] = 0;
    gb_pmsg_epg[ 5] = 0;
    gb_pmsg_epg[ 6] = 0;
    gb_pmsg_epg[ 7] = 0;
    gb_pmsg_epg[ 8] = 1;
    gb_pmsg_epg[ 9] = 0;
    gb_pmsg_epg[10] = 0;
    gb_pmsg_epg[11] = 0;
    gb_pmsg_epg[12] = 0;
    gb_pmsg_epg[13] = 0;
    gb_pmsg_epg[14] = 0;
    gb_pmsg_epg[15] = 0;
    gb_pmsg_epg[16] = 1;
    gb_pmsg_epg[17] = 1;
  #endif
#else
    fv_gctbl_init();
#endif

    return;
}

void fv_set_null(void)
{
    dwrd ld_bmu_base,ld_bmu_mate_base/*, ld_bmu_adr*/;

    //get free buffer ptr
    do
    {
        rb_cafp_set = CAFP_ACT | SRAM_BUF | CPUA_GET;
        while(rb_cafp_set & CAFP_ACT);
    } while(rb_cafp_rlt == 0);

    gw_null_ptr = rw_cafp_ptr;

    //set null pattern
    ld_bmu_base = SRAM_MEM_BASE + ((dwrd)(gw_null_ptr & 0x1ff) << 12);
    ld_bmu_mate_base = SRAM_MEM_BASE + ((dwrd)(gw_null_ptr & 0x1ff) << 12) + 0x200000;
    //for(ld_bmu_adr=0;ld_bmu_adr<(4096+64);ld_bmu_adr=ld_bmu_adr+4)
    //{
    //    //(*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) = 0xffffffff;
    //    (*(volatile dwrd *)(ld_bmu_base + ld_bmu_adr)) = 0x0;
    //}
    memset((byte *)(ld_bmu_base), 0x0, 4096);
    memset((byte *)(ld_bmu_mate_base ), 0x0, 64);

    return;
}

void fv_ftl_init(void)
{
    //pre-initial table variable
    fv_ftl_pre();

    //set null data pattern
    fv_set_null();

//#if (defined(QUICK_TABLE_FUN) || defined(PARTIAL_TABLE_FUN))
//
//#ifdef BAD_REMAP_FUN
//
//    //remap table
//    vFtlRemapTable();
//
//#endif
//
//    //build bad block bit map table
//    vFtlBadBlockBuild();
//
//#endif


//#ifdef QUICK_TABLE_FUN
//
//    //read table from NAND directly and check QBT_LABEL
//    if(bFtlQuickBuild(1))
//    {
//        fv_uart_print("quick build table sucessful\r\n");
//        return;
//    }
//    else
//    {
//        fv_uart_print("quick build table fail\r\n");
//    }
//
//#endif


#ifdef PARTIAL_TABLE_FUN

//    fv_uart_print("=======> %d.%d secs\r\n", rdwSystemTimer/CONFIG_TMR_TICK_HZ, 10*(rdwSystemTimer%CONFIG_TMR_TICK_HZ)/CONFIG_TMR_TICK_HZ);
//
//    //Serial Number Sorting
//    vFtlSNSort();
//
//    fv_uart_print("=======> %d.%d secs\r\n", rdwSystemTimer/CONFIG_TMR_TICK_HZ, 10*(rdwSystemTimer%CONFIG_TMR_TICK_HZ)/CONFIG_TMR_TICK_HZ);
//
//    //read spare byte to build Index table
//    vFtlIndexBuild();
//
//    fv_uart_print("=======> %d.%d secs\r\n", rdwSystemTimer/CONFIG_TMR_TICK_HZ, 10*(rdwSystemTimer%CONFIG_TMR_TICK_HZ)/CONFIG_TMR_TICK_HZ);
//
//    //read all table block to build VldCnt table
//    vFtlVldCntBuild();
//
//    fv_uart_print("=======> %d.%d secs\r\n", rdwSystemTimer/CONFIG_TMR_TICK_HZ, 10*(rdwSystemTimer%CONFIG_TMR_TICK_HZ)/CONFIG_TMR_TICK_HZ);
//
//    //read data page to build Cache table
//    vFtlCacheBuild();
//
//    fv_uart_print("=======> %d.%d secs\r\n", rdwSystemTimer/CONFIG_TMR_TICK_HZ, 10*(rdwSystemTimer%CONFIG_TMR_TICK_HZ)/CONFIG_TMR_TICK_HZ);

#else
    //erase build table
    fv_ers_build();
#endif


    //check the relationship of all table
    //vFtlTableCheck();

    return;
}

