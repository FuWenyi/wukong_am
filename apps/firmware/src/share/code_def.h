#ifndef _CODE_DEF_H
#define _CODE_DEF_H

/* ============ Param define ============ */
#define ENABLE      (1)
#define DISABLE     (0)
#define ENABLE_REVERSE      (0)
#define DISABLE_REVERSE     (1)

#define TRUE    (1)
#define FALSE   (0)

#define ERR    (-1)
#define OK      (0)

#ifndef FAIL
#define FAIL   (1)
#endif
#define SUCCESS (0)

#define STS_BUSY    (1)
#define STS_IDLE    (0)

/* ============ Universal computing ============ */
// get list member num
#define ITEM_OF(arr) (sizeof(arr) / sizeof(arr[0]))
// calculate err code first index
#define ERROR_CODE_CAL(idx) (((idx) << (8)) + 1)

#endif
