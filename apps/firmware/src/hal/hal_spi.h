#ifndef _HAL_SPI_H__
#define _HAL_SPI_H__


/*******************************************************************************************************/
/********************                 spic register definition               **************************/
/*******************************************************************************************************/
typedef unsigned char 	 	byte;
typedef unsigned short	 	word;
typedef unsigned int		dwrd;
typedef unsigned long long 	qwrd;

#define  SPI_REG_BASE    0x15600000


#define rb_ctrlr0_set     	(*(volatile dwrd    *)(SPI_REG_BASE+0x000))
  //FRF	
  #define MOTOROLA_SPI 		(0 << 4)
  #define TEXAS_SSP 		(1 << 4)
  #define NS_MICROWIRE 		(2 << 4)
  #define RESERVED 			(3 << 4)
  //SCPH
  #define SCPH_MIDDLE 		(0 << 6)
  #define SCPH_START 		(1 << 6)
  // SCPOL
  #define SCLK_LOW 			(0 << 7)
  #define SCLK_HIGH 		(1 << 7)
  // TMOD
  #define TX_AND_RX 		(0 << 8)
  #define TX_ONLY 			(1 << 8)
  #define RX_ONLY 			(2 << 8)
  #define EEPROM_READ 		(3 << 8)
  #define TMOD_MASK			(0xFFFFFCFF)
  // DFS_32
  #define DFS32_FRAME_04BITS 	(0x3 << 16) 
  #define DFS32_FRAME_08BITS 	(0x7 << 16)
  #define DFS32_FRAME_16BITS 	(0xf << 16)
  #define DFS32_FRAME_32BITS 	(0x1f << 16)
  // SPI_FRF
  #define STD_SPI_FRF 		(0 << 21)
  #define DUAL_SPI_FRF 		(1 << 21)
  #define QUAD_SPI_FRF 		(2 << 21)
  #define OCTAL_SPI_FRF 	(3 << 21)
  
#define rw_ctrlr1_set      	(*(volatile word    *)(SPI_REG_BASE+0x004))
#define rd_ssienr_set      	(*(volatile dwrd    *)(SPI_REG_BASE+0x008))
  #define DISABLES_SERIAL_TRANSFER 		(0x0)
  #define ENABLED_SERIAL_TRANSFER 		(0x1)

#define rd_mwcr_set      	(*(volatile dwrd    *)(SPI_REG_BASE+0x00c))
#define rd_ser_set      	(*(volatile dwrd    *)(SPI_REG_BASE+0x010))
  #define SLAVE_SELECT (0x1)

#define rw_baudr_set      	(*(volatile word    *)(SPI_REG_BASE+0x014))
#define rd_txftlr_set      	(*(volatile dwrd    *)(SPI_REG_BASE+0x018))
#define rd_rxftlr_set      	(*(volatile dwrd    *)(SPI_REG_BASE+0x01c))
#define rd_txflr_set      	(*(volatile dwrd    *)(SPI_REG_BASE+0x020))
#define rd_rxflr_set      	(*(volatile dwrd    *)(SPI_REG_BASE+0x024))
#define rd_sr_set      		(*(volatile dwrd    *)(SPI_REG_BASE+0x028))
  #define SPI_IS_BUSY	 	(1 << 0)
  #define TF_NOT_FULL 		(1 << 1)
  #define TF_IS_EMPTY 		(1 << 2)
  #define RF_NOT_EMPTY 		(1 << 3)
  #define RF_IS_FULL 		(1 << 4)
  #define TRANSMIT_ERR 		(1 << 5)

#define rd_imr_set      	(*(volatile dwrd    *)(SPI_REG_BASE+0x02c))
#define rd_isr_set      	(*(volatile dwrd    *)(SPI_REG_BASE+0x030))
#define rd_risr_set      	(*(volatile dwrd    *)(SPI_REG_BASE+0x034))
#define rd_idr_set      	(*(volatile dwrd    *)(SPI_REG_BASE+0x058))
#define rb_dr_set      		(*(volatile byte    *)(SPI_REG_BASE+0x060))
#define rd_rx_dly      		(*(volatile dwrd    *)(SPI_REG_BASE+0x0f0))
#define rd_ctrlr0_set      	(*(volatile dwrd    *)(SPI_REG_BASE+0x0f4))
  #define SPI_INST_ADDR_IN_S_MODE 		(0x0)
  #define SPI_ADDR_IN_FRF_MODE 			(0x1)
  #define SPI_INST_ADDR_IN_FRF_MODE 	(0x2)
  #define CTRLR0_MASK 					(0x3)


#define SPI_WAIT_BUSY_CNT (10000000)
#define SPI_READ_STATUS_CNT (0x1)
#define SPI_ERASE_NDF_CNT (4)
#define SPI_PAGE_CNT (256)

#define SPI_BLK_SIZE (64*1024)
#define SPI_BLK_MASK (SPI_BLK_SIZE-1)
#define SPI_SECTER_SIZE (4*1024)
#define SPI_SECTER_MASK (SPI_SECTER_SIZE-1)

// spi cmd op
#define SPI_PAGE_PROGRAM (0x2)
#define SPI_READ_DATA (0x3)
#define SPI_READ_STATUS (0x5)
#define SPI_WRITE_ENABLE (0x6)
#define SPI_SECTOR_4K_ERASE (0x20)
#define SPI_BLOCK_32K_ERASE (0x52)
#define SPI_BLOCK_64K_ERASE (0xD8)
#define SPI_CHIP_ERASE (0xC7)

void hal_spi_falsh_wait_busy(void);
void hal_spi_rx_cmd(dwrd cmd, dwrd ndf);
void hal_spi_tx_cmd(byte cmd);
void hal_spic_wait_busy(void);
void hal_spi_falsh_wait_busy(void);
void hal_spi_init(void);
void hal_spi_chip_erase(void);
void hal_spi_sector_erase(dwrd sector_addr);
void hal_spi_block_erase(dwrd block_addr);
/* The buffer size pointed to by ram_data_addr must be an integer multiple of 256 bytes.  */
void hal_spi_read(dwrd ram_data_addr, dwrd norflash_data_addr, dwrd data_len);
void hal_spi_write(dwrd ram_data_addr, dwrd norflash_data_addr, dwrd data_len);

/* Read information from nor in bytes; It is recommended to use this interface when the number of bytes read is less than 256. */
void hal_spi_byte_read(dwrd src_data_addr, dwrd dst_data_addr, dwrd data_len);
void hal_spi_page_program(dwrd src_data_addr, dwrd dst_data_addr, dwrd data_len);
void hal_spi_erase(dwrd start_addr, dwrd len);

#endif

