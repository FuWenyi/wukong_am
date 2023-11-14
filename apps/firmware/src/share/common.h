#ifndef _SHARE_COMMON_H_
#define _SHARE_COMMON_H_

#define LBA_4K // If other LBA lengths are supported in the future, a new configuration item will be added
//#define UART_DBG

#define RWCMD_THR   16
#define HMETA_NUM   g_configs_in_dtcm.host_meta_len

#define BYTE_SHIFT  8
#define WORD_SHIFT  16
#define DWRD_SHIFT  32

#define BYTE_MASK   0xff
#define WORD_MASK   0xffff
#define DWRD_MASK   0xffffffff

#define DBLK_INVLD  0xffff

#define SPI_NOR_SPOR_PRESIST_DATA_START_ADDR  (0x100000)
#define SPI_NOR_SPOR_PRESIST_DATA_LEN         (4096)

/**
  * get bit(s) from a value
  * e.g uint32_t value = 0x12345678, obtain bits [6:4]
  *     uint8_t result = GET_BITS(value,4,6) = 7
  * @param data  src value
  * @param start start bit, should not be negative!
  * @param end   end   bit, should not be negative!
  * @return value of bits
  */
#define GET_BITS(data, start, end) (((data) >> (start)) & ((1LL << ((end) - (start) + 1)) - 1))

  /**
   * set bit value to a specific data
   *
   * @param data  the src and target data
   * @param start start bit, should not be negative!
   * @param end   end   bit, should not be negative!
   * @param value value of bits, don't worry about if the value
   *              greater than the max range of the bists
   */
#define SET_BITS(data, start, end, value) \
    do { \
        uint64_t mask = ((1ull << ((end) - (start) + 1)) - 1ull) << (start); \
        data = ((data) & ~mask) | (( ((uint64_t)(value)) << (start)) & mask); \
    } while (0)

#define GET_L32_OF_U64(num) ((uint32_t)(num))
#define GET_H32_OF_U64(num) ((uint32_t)((num) >> 32))

#endif
