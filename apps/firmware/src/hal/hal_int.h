#ifndef _INTC_H_
#define _INTC_H_

typedef unsigned char 	 	byte;
typedef unsigned short	 	word;
typedef unsigned int		dwrd;
typedef unsigned long long 	qwrd;

#define INTC_REG_BASE    0x15100000

typedef struct _IRQ_FINALSTATUS_L
{
  union
  {
	  dwrd whole;
	  struct
	  {
		  volatile dwrd uart_status : 1;
		  volatile dwrd timer_status : 1;
		  volatile dwrd wdt_status : 1;
		  volatile dwrd ssi_status : 1;
		  volatile dwrd i2c_status : 12;
		  volatile dwrd gpio_status : 16;
	  } ctrl;
  } ic_irq_finalsts_l;
}IRQ_FINALSTATUS_L, *PIRQ_FINALSTATUS_L;

enum IRQ_Index {
    IRQ_Uart_Int = 0,
    IRQ_Timer_Int = 1,
    IRQ_Wdt_Int = 2,
    IRQ_Ssi_Int = 3,
    IRQ_I2C_Start_det_Int = 4,
    IRQ_I2C_Stop_det_Int = 5,
    IRQ_I2C_Activity_Int = 6,
    IRQ_I2C_Rd_Req_Int = 7,
    IRQ_I2C_Tx_Empty_Int = 8,
    IRQ_I2C_Tx_Over_Int = 9,
    IRQ_I2C_Tx_Abrt_Int = 10,
    IRQ_I2C_Rx_Done_Int = 11,
    IRQ_I2C_Rx_Full_Int = 12,
    IRQ_I2C_Rx_Over_Int = 13,
    IRQ_I2C_Rx_Under_Int = 14,
    IRQ_I2C_Gen_Call_Int = 15,
    IRQ_GPIO17_Int = 16,
    IRQ_GPIO18_Int = 17,
    IRQ_GPIO19_Int = 18,
    IRQ_GPIO20_Int = 19,
    IRQ_GPIO21_Int = 20,
    IRQ_GPIO22_Int = 21,
    IRQ_GPIO23_Int = 22,
    IRQ_GPIO24_Int = 23,
    IRQ_GPIO25_Int = 24,
    IRQ_GPIO26_Int = 25,
    IRQ_GPIO27_Int = 26,
    IRQ_GPIO28_Int = 27,
    IRQ_GPIO29_Int = 28,
    IRQ_GPIO30_Int = 28,
    IRQ_GPIO31_Int = 30,
    IRQ_GPIO0_Int = 31,
};

typedef void (*IpIsrFunc)(void);

enum IntcIntrPrio {
    //	Intc_FIQ0 = 0,		//FIQ; FIQ's higher priority
    //	Intc_FIQ1 = 1,		//FIQ; FIQ's lower priority
    Intc_IRQ0 = 2,  // IRQ; IRQ's highest priority
    Intc_IRQ1 = 3,  // IRQ; IRQ's 2nd highest priority
    Intc_IRQ2 = 4,
    Intc_IRQ3 = 5,
    Intc_IRQ4 = 6,
    Intc_IRQ5 = 7,  // IRQ; IRQ's lowest priority
};

typedef enum IntcIntrPrio IntcIntrPrio;

#define rd_int_l      			(*(volatile dwrd    *)(INTC_REG_BASE+0x0))
#define rd_int_h       			(*(volatile dwrd    *)(INTC_REG_BASE+0x4))
#define rd_intmask_l    		(*(volatile dwrd    *)(INTC_REG_BASE+0x8))
#define rd_intmask_h    		(*(volatile dwrd    *)(INTC_REG_BASE+0xc))
#define rd_intforce_l      		(*(volatile dwrd    *)(INTC_REG_BASE+0x10))
#define rd_intforce_h      		(*(volatile dwrd    *)(INTC_REG_BASE+0x14))
#define rd_rawsts_l      		(*(volatile dwrd    *)(INTC_REG_BASE+0x18))
#define rd_rawsts_h				(*(volatile dwrd    *)(INTC_REG_BASE+0x1c))
#define rd_sts_l				(*(volatile dwrd    *)(INTC_REG_BASE+0x20))
#define rd_sts_h				(*(volatile dwrd    *)(INTC_REG_BASE+0x24))
#define rd_masksts_l	   	 	(*(volatile dwrd    *)(INTC_REG_BASE+0x28))
#define rd_masksts_h			(*(volatile dwrd    *)(INTC_REG_BASE+0x2c))
#define rd_finsts_l				((volatile IRQ_FINALSTATUS_L    *)(INTC_REG_BASE+0x30))
#define rd_finsts_h				(*(volatile dwrd    *)(INTC_REG_BASE+0x34))
#define rd_vector_set			(*(volatile dwrd    *)(INTC_REG_BASE+0x38))
#define rd_vector_n(N)			(*(volatile dwrd    *)(INTC_REG_BASE+0x40+8*N))
#define rd_fiq_intr				(*(volatile dwrd    *)(INTC_REG_BASE+0xc0))
#define rd_fiq_intrmask			(*(volatile dwrd    *)(INTC_REG_BASE+0xc4))
#define rd_fiq_intforce			(*(volatile dwrd    *)(INTC_REG_BASE+0xc8))
#define rd_fiq_rawsts			(*(volatile dwrd    *)(INTC_REG_BASE+0xcc))
#define rd_fiq_sts				(*(volatile dwrd    *)(INTC_REG_BASE+0xd0))
#define rd_fiq_finsts			(*(volatile dwrd    *)(INTC_REG_BASE+0xd4))
#define rd_irq_lev				(*(volatile dwrd    *)(INTC_REG_BASE+0xd8))
#define rd_irq_pr(N)      		(*(volatile dwrd    *)(INTC_REG_BASE+0xe8+4*N))  

#define INTC_NUM_OF_CHANNELS 32 /* number of channels */

#define IRQ_DISABLE      0x0
#define IRQ_INIT_MASK    0xFFFF
#define IRQ_SYS_PRIORITY 0x0

// interrupt sources
#define IRQ_UART  (0)
#define IRQ_TIMER (1)
#define IRQ_WDT   (2)
#define IRQ_SSI   (3)
#define IRQ_I2C   (4)
#define IRQ_GPIO  (5)

#define IRQ_UART_NUM  (1)
#define IRQ_TIMER_NUM (1)
#define IRQ_WDT_NUM   (1)
#define IRQ_SSI_NUM   (1)
#define IRQ_I2C_NUM   (12)
#define IRQ_GPIO_NUM  (16)


void hal_intc_isr_fiq(void);
void hal_intc_isr_irq(void);
void hal_intc_init(void);
byte hal_intc_reg_isr(byte bChnlNum, IntcIntrPrio nPrio, IpIsrFunc isr);
byte hal_intc_irq_isr(byte bChnlNum, byte nPrio, IpIsrFunc isr);
void hal_intc_unreg_isr(byte bChnlNum);
void hal_intc_en_intr(byte bChnlNum);
void hal_intc_disable_intr(byte bChnlNum);
void hal_Intc_disable_allintrs(void);
void hal_def_irq_isr(void);
void hal_isr_irq(void);

#endif

