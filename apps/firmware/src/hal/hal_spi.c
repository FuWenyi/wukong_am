#include "hal_spi.h"
#include "string.h"

/* These variables are only used in the current file, so they are moved here from the external Header file. */

void hal_spi_init(void)
{
    rd_ssienr_set &= DISABLES_SERIAL_TRANSFER;
	rb_ctrlr0_set = (rb_ctrlr0_set & 0x100FC0F) | MOTOROLA_SPI | SCPH_START | SCLK_LOW | DFS32_FRAME_08BITS | STD_SPI_FRF;
    rw_baudr_set = 0x10;
    rb_ctrlr0_set = (rb_ctrlr0_set & ~CTRLR0_MASK) | SPI_INST_ADDR_IN_S_MODE;
    rd_ser_set |= SLAVE_SELECT;
}

void hal_spi_rx_cmd(dwrd cmd, dwrd ndf)
{
    rd_ssienr_set &= DISABLES_SERIAL_TRANSFER;
	rb_ctrlr0_set = (rb_ctrlr0_set & TMOD_MASK) | EEPROM_READ;
    rw_ctrlr1_set = ndf - 1;
    rd_ssienr_set |= ENABLED_SERIAL_TRANSFER;

    rb_dr_set = cmd;

    hal_spic_wait_busy();
}

void hal_spi_tx_cmd(byte cmd)
{
    rd_ssienr_set &= DISABLES_SERIAL_TRANSFER;
	rb_ctrlr0_set = (rb_ctrlr0_set & TMOD_MASK) | TX_ONLY;
    rd_ssienr_set |= ENABLED_SERIAL_TRANSFER;

    rb_dr_set = cmd;

    hal_spic_wait_busy();

    hal_spi_falsh_wait_busy();
}

void hal_spic_wait_busy(void)
{
    // check SR until SR[0] is 1(SPIC is not busy)
    dwrd delay_cnt = 0, tmp_cnt;
    while (1) 
	{
        if(rd_sr_set & TRANSMIT_ERR) 
		{
            break;
        }
        else 
		{
            if(!(rd_sr_set & SPI_IS_BUSY)) 
			{
                break;
            }
        }

        for(tmp_cnt = 0; tmp_cnt < 0xff; tmp_cnt++) 
		{
            __asm__ volatile(
                "nop;");
        }

        delay_cnt += 1;
        if(delay_cnt > SPI_WAIT_BUSY_CNT) 
		{
            //uart_print_direct("spic wait busy timeout\r\n");
            break;
        }
    }
}

void hal_spi_falsh_wait_busy(void)
{
    dwrd delay_cnt = 0, tmp_cnt;

    while (1) 
	{
        hal_spi_rx_cmd(SPI_READ_STATUS, SPI_READ_STATUS_CNT);  // 05H -> RDSR

        // SPIC read DR0: RDSR
        if (!(rb_dr_set & SPI_IS_BUSY)) 
		{
            break;
        }

        for (tmp_cnt = 0; tmp_cnt < 0xff; tmp_cnt++) 
		{
            __asm__ volatile(
                "nop;");
        }

        delay_cnt += 1;
        if (delay_cnt > SPI_WAIT_BUSY_CNT) 
		{
            //uart_print_direct("falsh wait busy timeout\r\n");
            break;
        }
    }
}

void hal_spi_addr_erase(byte cmd, dwrd addr)
{
	dwrd cnt;

    hal_spi_tx_cmd(SPI_WRITE_ENABLE);

    rd_ssienr_set &= DISABLES_SERIAL_TRANSFER;
	rb_ctrlr0_set = (rb_ctrlr0_set & TMOD_MASK) | TX_ONLY;
    rd_ssienr_set |= ENABLED_SERIAL_TRANSFER;

    for (cnt = 0; cnt < 4; cnt += 1) 
	{
        if (0 == cnt)
            rb_dr_set = cmd;
        else
            rb_dr_set = (addr >> ((3 - cnt) << 3)) & 0xFF;
    }

    hal_spic_wait_busy();

    hal_spi_falsh_wait_busy();
}

void hal_spi_sector_erase(dwrd sector_addr)
{
    hal_spi_addr_erase(SPI_SECTOR_4K_ERASE, sector_addr);
}

void hal_spi_block_erase(dwrd block_addr)
{
    hal_spi_addr_erase(SPI_BLOCK_64K_ERASE, block_addr);
}

void hal_spi_chip_erase(void)
{
    hal_spi_tx_cmd(SPI_WRITE_ENABLE);
    hal_spi_tx_cmd(SPI_CHIP_ERASE);
}

void hal_spi_erase(dwrd start_addr, dwrd len)
{

    dwrd end_addr_4k = (start_addr + len + SPI_SECTER_MASK)>>12 << 12 ;
    dwrd start_addr_4k = (start_addr>>12)<< 12;
    dwrd first_blk_erase_num_4k;
    byte i;

    if(len < SPI_BLK_SIZE)
	{
    	first_blk_erase_num_4k = (end_addr_4k - start_addr_4k) >> 12;
    }
    else
	{
    	first_blk_erase_num_4k = (SPI_BLK_SIZE -  start_addr_4k & SPI_BLK_MASK) >> 12;

        dwrd end_addr_start_4k = end_addr_4k&(~SPI_BLK_MASK);// divid 64
        dwrd end_blk_erase_num_4k = (end_addr_4k & SPI_BLK_MASK)>> 12;
        for(i = 0; i < end_blk_erase_num_4k; i++)
		{
            hal_spi_sector_erase(end_addr_start_4k + i*SPI_SECTER_SIZE);
        }

        dwrd start_addr_64k = ((start_addr_4k + SPI_BLK_MASK)>>16) << 16;// divid 64
        dwrd end_addr_64k = (end_addr_4k>>16)<< 16;// divid 64
        dwrd erase_num_64k = (end_addr_64k - start_addr_64k)>>16;// divid 64
        for(i = 0; i < erase_num_64k; i++)
		{
            hal_spi_sector_erase(start_addr_64k + i*SPI_BLK_SIZE);
        }
    }

    for(i = 0; i < first_blk_erase_num_4k; i++)
	{
        hal_spi_sector_erase(start_addr_4k + i*SPI_SECTER_SIZE);
    }
}


/* Read information from nor in bytes; It is recommended to use this interface when the number of bytes read is less than 256. */
void hal_spi_byte_read(dwrd src_data_addr, dwrd dst_data_addr, dwrd data_len)
{
    dwrd cnt;

    rd_ssienr_set &= DISABLES_SERIAL_TRANSFER;
	rb_ctrlr0_set = (rb_ctrlr0_set & TMOD_MASK) | EEPROM_READ;
    rw_ctrlr1_set = data_len - 1;
    rd_ssienr_set |= ENABLED_SERIAL_TRANSFER;

    for (cnt = 0; cnt < 4; cnt += 1) 
	{
        if (0 == cnt)
            rb_dr_set = SPI_READ_DATA;
        else
            rb_dr_set = (src_data_addr >> ((3 - cnt) << 3)) & 0xFF;
    }

    for (cnt = 0; cnt < data_len; cnt += 1) 
	{
        while ((rd_sr_set & RF_NOT_EMPTY) == 0x0)
            ;
        (*((volatile byte *)(dst_data_addr + cnt))) = rb_dr_set;
    }

    hal_spic_wait_busy();

    hal_spi_falsh_wait_busy();
}

/* The buffer size pointed to by ram_data_addr must be an integer multiple of 256 bytes.  */
void hal_spi_read(dwrd ram_data_addr, dwrd norflash_data_addr, dwrd data_len)
{
    dwrd transter_page_cnt;
    dwrd tmp_src_addr = norflash_data_addr;
    dwrd tmp_dst_addr = ram_data_addr;
    dwrd current_transter_cnt = 0;

    for (transter_page_cnt = 0; transter_page_cnt < (data_len / SPI_PAGE_CNT); transter_page_cnt++) 
	{
        current_transter_cnt = transter_page_cnt * SPI_PAGE_CNT;
        hal_spi_byte_read(tmp_src_addr + current_transter_cnt, tmp_dst_addr + current_transter_cnt, SPI_PAGE_CNT);
    }
    if (data_len % SPI_PAGE_CNT) 
	{
        current_transter_cnt = transter_page_cnt * SPI_PAGE_CNT;
        hal_spi_byte_read(tmp_src_addr + current_transter_cnt, tmp_dst_addr + current_transter_cnt, SPI_PAGE_CNT);
    }
}

void hal_spi_page_program(dwrd src_data_addr, dwrd dst_data_addr, dwrd data_len)
{
    dwrd cnt;

    hal_spi_tx_cmd(SPI_WRITE_ENABLE);

    rd_ssienr_set &= DISABLES_SERIAL_TRANSFER;
	rb_ctrlr0_set = (rb_ctrlr0_set & TMOD_MASK) | TX_ONLY;
    rd_ssienr_set |= ENABLED_SERIAL_TRANSFER;

    for (cnt = 0; cnt < 4; cnt += 1) 
	{
        if (0 == cnt)
            rb_dr_set = SPI_PAGE_PROGRAM;
        else
            rb_dr_set = (dst_data_addr >> ((3 - cnt) << 3)) & 0xFF;
    }

    for (cnt = 0; cnt < data_len; cnt += 1) 
	{
        while ((rd_sr_set & TF_NOT_FULL) == 0x0)
            ;
        rb_dr_set = (*((volatile byte *)(src_data_addr + cnt)));
    }

    while (!(rd_sr_set & TF_IS_EMPTY))
        ;

    hal_spic_wait_busy();

    hal_spi_falsh_wait_busy();
}

void hal_spi_write(dwrd ram_data_addr, dwrd norflash_data_addr, dwrd data_len)
{
    dwrd transter_page_cnt;
    dwrd tmp_src_addr = ram_data_addr;
    dwrd tmp_dst_addr = norflash_data_addr;
    dwrd current_transter_cnt = 0;

    for (transter_page_cnt = 0; transter_page_cnt < (data_len / SPI_PAGE_CNT); transter_page_cnt++) 
	{
        current_transter_cnt = transter_page_cnt * SPI_PAGE_CNT;
        hal_spi_page_program(tmp_src_addr + current_transter_cnt, tmp_dst_addr + current_transter_cnt, SPI_PAGE_CNT);
    }
    if (data_len % SPI_PAGE_CNT) 
	{
        current_transter_cnt = transter_page_cnt * SPI_PAGE_CNT;
        memset((byte *)(tmp_src_addr + current_transter_cnt + data_len % SPI_PAGE_CNT), 0xFF, (SPI_PAGE_CNT - data_len % SPI_PAGE_CNT));
        hal_spi_page_program(tmp_src_addr + current_transter_cnt, tmp_dst_addr + current_transter_cnt, SPI_PAGE_CNT);
    }
}

