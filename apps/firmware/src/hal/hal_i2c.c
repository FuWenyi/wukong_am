#include "hal_i2c.h"

void hal_i2c_init(void)
{
    rd_ic_enable_set.enable = DISABLE_GENERAL;

    rd_ic_con.ic_slave_disable = ENABLE_GENERAL;
    rd_ic_con.ic_restart_en = ENABLE_GENERAL;
    rd_ic_con.ic_10bitaddr_master = ADDR_7BITS_MODE;
    rd_ic_con.ic_10bitaddr_slave = ADDR_7BITS_MODE;
    rd_ic_con.speed = SPEED_STANDARD;
    rd_ic_con.master_mode = ENABLE_GENERAL;

    rd_ic_tar.ic_tar = MP5515_ADDR;
    rd_ic_repeat_h = SCLK_100K_HCNT;
    rd_ic_repeat_l = SCLK_100K_LCNT;

    rd_ic_intr_mask = ENABLE_CONTRARY;
    rd_ic_rx_tl.rx_tl = THRESHOLD_LEVEL;
    rd_ic_tx_tl.tx_tl = THRESHOLD_LEVEL;
    // current_i2c_register->ic_dma_cr.ctrl.tdmae = ENABLE_GENERAL;
    // current_i2c_register->ic_dma_tdlr.ctrl.dmatdl = ENABLE_GENERAL;

    rd_ic_enable_set.enable = ENABLE_GENERAL;
}

void hal_i2c_wait_busy()
{
    dwrd delay_cnt = 0, tmp_cnt;
    while (1) 
    {
        if (rd_ic_sts_set.tfe && (!rd_ic_sts_set.activity)) 
        {
            break;
        }

        for (tmp_cnt = 0; tmp_cnt < 0x1ff; tmp_cnt++) 
        {
            __asm__ volatile(
                "nop;");
        }

        delay_cnt += 1;
        if (delay_cnt > I2C_WAIT_BUSY_CNT) 
        {
            //uart_print_direct("i2c wait busy timeout\r\n");
            break;
        }
    }
}

void hal_i2c_write(byte addr, byte data)
{
    // send: slave address & register & write cmd
    rd_ic_data_cmd->ic_data_cmd.whole = (CMD_WRITE << 8) | addr;
    // send: write data
    rd_ic_data_cmd->ic_data_cmd.ctrl.dat = data;

    hal_i2c_wait_busy();
}

byte hal_i2c_read(byte addr)
{
    // send: register address & write cmd
    rd_ic_data_cmd->ic_data_cmd.whole = (CMD_WRITE << 8) | addr;
    while (rd_ic_sts_set.rfne == 0x1);

    // send: read cmd
    rd_ic_data_cmd->ic_data_cmd.whole = (CMD_READ << 8);

    hal_i2c_wait_busy();

    return rd_ic_data_cmd->ic_data_cmd.ctrl.dat;
}

void hal_i2c_idle_to_disable(void)
{
    while (rd_ic_sts_set.mst_activity == ACT_ACTIVE)
        ;
    rd_ic_enable_set.enable = DISABLE_GENERAL;
}

