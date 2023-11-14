#ifndef _HAL_I2C_H
#define _HAL_I2C_H

typedef unsigned char 	 	byte;
typedef unsigned short	 	word;
typedef unsigned int		dwrd;
typedef unsigned long long 	qwrd;

#define I2C_REG_BASE 0x15700000

typedef struct _IC_CON
{
  dwrd master_mode : 1; 		// 0x1:enable 0x0:disable
  dwrd speed : 2;				// 0x1:standard 0x2:fast 0x3:high
  dwrd ic_10bitaddr_slave : 1;	// 0x1:addr_10bits 0x0:addr_7bits
  dwrd ic_10bitaddr_master : 1; // 0x1:addr_10bits 0x0:addr_7bits
  dwrd ic_restart_en : 1;		// 0x1:enable 0x0:disable
  dwrd ic_slave_disable : 1;	// 0x1:slave_enable 0x0:slave_disable
  dwrd stop_det_ifaddressed : 1;
  dwrd tx_empty_ctrl : 1;
  dwrd rx_fifo_full_hld_ctrl : 1;
  dwrd stop_det_if_master_active : 1;
  dwrd bus_clear_feature_ctrl : 1;
  dwrd rsvd_ic_con_1 : 4;
  dwrd optionnal_sar_ctrl : 1;
  dwrd smbus_slave_quick_en : 1;
  dwrd smbus_app_en : 1;
  dwrd smbus_persistent_slv_addr_en : 1;
  dwrd rsvd_ic_con_2 : 12;
}IC_CON, *PIC_CON;

typedef struct _IC_ENABLE
{
	dwrd enable : 1; // 0x1:enable 0x0:disable
	dwrd abort : 1;
	dwrd tx_cmd_block : 1;
	dwrd sda_stuck_recovery_enble : 1;
	dwrd rsvd_ic_enable_1 : 12;
	dwrd smbus_clk_reset : 1;
	dwrd smbus_suspend_en : 1;
	dwrd smbus_alert_en : 1;
	dwrd rsvd_ic_enable_2 : 13;
}IC_ENABLE, *PIC_ENABLE;

typedef struct _IC_TAR
{
  dwrd ic_tar : 10;
  dwrd gc_or_start : 1;
  dwrd special : 1;
  dwrd ic_10bitaddr_master : 1;
  dwrd device_id : 1;
  dwrd rsvd_ic_tar_1 : 2;
  dwrd smbus_quick_cmd : 1;
  dwrd rsvd_ic_tar_2 : 15;
}IC_TAR, *PIC_TAR;

typedef struct _IC_RX_TL
{
	dwrd rx_tl : 8; // value rang:0~255
	dwrd rsvd_ic_rx_tl : 24;
}IC_RX_TL, *PIC_RX_TL;

typedef struct _IC_TX_TL
{
	dwrd tx_tl : 8; // value rang:0~255
	dwrd rsvd_ic_tx_tl : 24;
}IC_TX_TL, *PIC_TX_TL;

typedef struct _DATA_CMD
{
  union
  {
	  dwrd whole;
	  struct
	  {
		  dwrd dat : 8;
		  dwrd cmd : 1; 			// 0x1:read 0x0:write
		  dwrd stop : 1;			// 0x1:enable 0x0:disable
		  dwrd restart : 1; 		// 0x1:enable 0x0:disable
		  dwrd first_data_byte : 1; // 0x1:active 0x0:inactive
		  dwrd rsvd_ic_data_cmd : 20;
	  } ctrl;
  } ic_data_cmd;
}DATA_CMD, *PDATA_CMD;

typedef struct _IC_STS
{
  dwrd activity : 1;
  dwrd tfnf : 1;		 // Tx FIFO status: 0x1:not_full 0x0:full
  dwrd tfe : 1; 		 // Tx FIFO status: 0x1:empty 0x0:not_empty
  dwrd rfne : 1;		 // Rx FIFO status: 0x1:not_empty 0x0:empty
  dwrd rff : 1; 		 // Rx FIFO status: 0x1:full 0x0:not_full
  dwrd mst_activity : 1; // 0x1:active 0x0:idle
  dwrd slv_activity : 1; // 0x1:active 0x0:idle
  dwrd mst_hold_tx_fifo_empty : 1;
  dwrd mst_hold_rx_fifo_full : 1;
  dwrd slv_hold_tx_fifo_empty : 1;
  dwrd slv_hold_rx_fifo_full : 1;
  dwrd sda_stuck_not_recovered : 1;
  dwrd rsvd_ic_status_1 : 4;
  dwrd smbus_quick_cmd_bit : 1;
  dwrd smbus_slave_addr_valid : 1;
  dwrd smbus_slave_addr_resolved : 1;
  dwrd smbus_suspend_status : 1;
  dwrd smbus_alert_status : 1;
  dwrd rsvd_ic_status_2 : 11;

} IC_STS, *PIC_STS;

#define rd_ic_con       	(*(volatile IC_CON    *)(I2C_REG_BASE+0x0))
#define rd_ic_tar      	    (*(volatile IC_TAR    *)(I2C_REG_BASE+0x4))
#define rd_ic_sar        	(*(volatile dwrd    *)(I2C_REG_BASE+0x8))
#define rd_ic_hs_maddr      	(*(volatile dwrd    *)(I2C_REG_BASE+0xc))
#define rd_ic_data_cmd      	((volatile DATA_CMD    *)(I2C_REG_BASE+0x10))
#define rd_ic_repeat_h      	(*(volatile dwrd    *)(I2C_REG_BASE+0x14))
#define rd_ic_repeat_l      	(*(volatile dwrd    *)(I2C_REG_BASE+0x18))
#define rd_ic_repeat_other		(*(volatile dwrd    *)(I2C_REG_BASE+0x1c))
#define rd_ic_fs_scl_l			(*(volatile dwrd    *)(I2C_REG_BASE+0x20))
#define rd_ic_hs_scl_h			(*(volatile dwrd    *)(I2C_REG_BASE+0x24))
#define rd_ic_hs_scl_l			(*(volatile dwrd    *)(I2C_REG_BASE+0x28))
#define rd_ic_intr_stat		(*(volatile dwrd    *)(I2C_REG_BASE+0x2c))
#define rd_ic_intr_mask		(*(volatile dwrd    *)(I2C_REG_BASE+0x30))
#define rd_ic_raw_intr_stat		(*(volatile dwrd    *)(I2C_REG_BASE+0x34))
#define rd_ic_rx_tl			(*(volatile IC_RX_TL    *)(I2C_REG_BASE+0x38))
#define rd_ic_tx_tl			(*(volatile IC_TX_TL    *)(I2C_REG_BASE+0x3c))
#define rd_ic_intr_set			(*(volatile dwrd    *)(I2C_REG_BASE+0x40))
#define rd_ic_rx_under			(*(volatile dwrd    *)(I2C_REG_BASE+0x44))
#define rd_ic_rx_over			(*(volatile dwrd    *)(I2C_REG_BASE+0x48))
#define rd_ic_tx_over			(*(volatile dwrd    *)(I2C_REG_BASE+0x4c))
#define rd_ic_rd_ic_req			(*(volatile dwrd    *)(I2C_REG_BASE+0x50))
#define rd_ic_tx_abrt			(*(volatile dwrd    *)(I2C_REG_BASE+0x54))
#define rd_ic_rx_done			(*(volatile dwrd    *)(I2C_REG_BASE+0x58))
#define rd_ic_activity_set		(*(volatile dwrd    *)(I2C_REG_BASE+0x5c))
#define rd_ic_stop_det			(*(volatile dwrd    *)(I2C_REG_BASE+0x60))
#define rd_ic_start_det		(*(volatile dwrd    *)(I2C_REG_BASE+0x64))
#define rd_ic_gen_call			(*(volatile dwrd    *)(I2C_REG_BASE+0x68))
#define rd_ic_enable_set		(*(volatile IC_ENABLE    *)(I2C_REG_BASE+0x6c))
#define rd_ic_sts_set			(*(volatile IC_STS    *)(I2C_REG_BASE+0x70))
#define rd_ic_txflr_set		(*(volatile dwrd    *)(I2C_REG_BASE+0x74))
#define rd_ic_rxflr_set		(*(volatile dwrd    *)(I2C_REG_BASE+0x78))
#define rd_ic_sda_hold			(*(volatile dwrd    *)(I2C_REG_BASE+0x7c))
#define rd_ic_tx_abrt_s		(*(volatile dwrd    *)(I2C_REG_BASE+0x80))
#define rd_ic_slv_data			(*(volatile dwrd    *)(I2C_REG_BASE+0x84))
#define rd_ic_dma_cr			(*(volatile dwrd    *)(I2C_REG_BASE+0x88))
#define rd_ic_dma_tdlr			(*(volatile dwrd    *)(I2C_REG_BASE+0x8c))
#define rd_ic_dma_trlr			(*(volatile dwrd    *)(I2C_REG_BASE+0x90))
#define rd_ic_sda_setup		(*(volatile dwrd    *)(I2C_REG_BASE+0x94))				
#define rd_ic_ack_gen			(*(volatile dwrd    *)(I2C_REG_BASE+0x98))	
#define rd_ic_en_sts			(*(volatile dwrd    *)(I2C_REG_BASE+0x9c))		
#define rd_ic_fs_spkl			(*(volatile dwrd    *)(I2C_REG_BASE+0xa0))				
#define rd_ic_optional_sar		(*(volatile dwrd    *)(I2C_REG_BASE+0xd8))		


#define ENABLE_GENERAL  0x1
#define DISABLE_GENERAL 0x0

#define ENABLE_CONTRARY  0x0
#define DISABLE_CONTRARY 0x1

#define ADDR_10BITS_MODE 0X1
#define ADDR_7BITS_MODE  0X0

#define SPEED_STANDARD 0x1
#define SPEED_FAST     0x2
#define SPEED_HIGH     0x3

#define CMD_READ  0x1
#define CMD_WRITE 0x0

#define STATUS_EMPTY     0x1
#define STATUS_NOT_EMPTY 0x0
#define STATUS_FULL      0x1
#define STATUS_NOT_FULL  0x0

#define ACT_ACTIVE 0x1
#define ACT_IDLE   0x0

#define THRESHOLD_LEVEL 0X3f
#define MP5515_ADDR     0x33
#define SCLK_100K_HCNT  400
#define SCLK_100K_LCNT  470

#define I2C_WAIT_BUSY_CNT 10000

/**
 * @brief  disponse ic config
 * @note   all the info is defualt. the config order should be hold
 * @retval None
 */
void hal_i2c_init(void);

/**
 * @brief  write cmd flow
 * @note   defualt slave is : MP5515 address
 * @param  addr: register address
 * @param  data: data to write regitser
 * @retval None
 */
void hal_i2c_write(byte addr, byte data);

/**
 * @brief  read cmd flow
 * @note   defualt slave is : MP5515 address
 * @param  addr: register address
 * @retval read data
 */
byte hal_i2c_read(byte addr);

/**
 * @brief  ic close
 * @note   if ic status is idle, disable ic
 * @retval None
 */
void hal_i2c_idle_to_disable(void);

#endif  // _DW_APP_I2C_H

