#ifndef _TEST_MISC_H_
#define _TEST_MISC_H_


typedef unsigned char 	 	byte;
typedef unsigned short	 	word;
typedef unsigned int		dwrd;
typedef unsigned long long 	qwrd;

dwrd hal_spi_test(void);
dwrd hal_gpio_test(void);
dwrd hal_i2c_test(void);
dwrd hal_hareware_timer_test(void);


#endif
