#ifndef _HAL_ERR_CODE_H
#define _HAL_ERR_CODE_H

/* ============ Param define ============ */
#define ENABLE      (1)
#define DISABLE     (0)
#define ENABLE_REVERSE      (0)
#define DISABLE_REVERSE     (1)

#define TRUE    (1)
#define FALSE   (0)

#define RET_ERR    (0xffffffff)
#define RET_OK     (0)

// #ifndef FAIL
// #define FAIL   (1)
// #endif
// #ifndef SUCCESS
// #define SUCCESS (0)
// #endif

#define STS_BUSY    (1)
#define STS_IDLE    (0)

/* ============ Universal computing ============ */
// get list member num
#define ITEM_OF(arr) (sizeof(arr) / sizeof(arr[0]))

#endif
