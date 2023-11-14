#ifndef _P4K_H
#define _P4K_H

#include <stdint.h>
#include "../inc/global.h"

typedef struct
{
    uint16_t blk;
    uint16_t first_page;
    uint8_t bank;
    uint8_t ch;
    uint8_t plane;
    uint16_t rd_num;
} UNIT_PARAM;

//#define FRAG_MASK   ((1 << FRAG_SHIFT) - 1)
#define CHAN_MASK   ((1 << CHAN_SHIFT) - 1)
#define PCE_MASK    ((1 << PCE_SHIFT) - 1)
#define LUN_MASK    ((1 << BANK_SHIFT) - 1)
#define BANK_MASK   ((1 << BANK_SHIFT) - 1)
//#define PAGE_MASK   ((1 << PAGE_SHIFT) - 1)
#define BLK_MASK    ((1 << BLK_SHIFT) - 1)
#define PLANE_MASK  ((1 << PLANE_SHIFT) - 1)

#define BKCH_SHIFT       (BANK_SHIFT + CHAN_SHIFT)
#define CH_FRAG_SHIFT    (CHAN_SHIFT + FRAG_SHIFT)
#define CE_CH_FRAG_SHIFT (PCE_SHIFT + CHAN_SHIFT + FRAG_SHIFT)
#define BKCH_FRAG_SHIFT  (BANK_SHIFT + CHAN_SHIFT + FRAG_SHIFT)

#define phy_to_p4k(phy_adr) (uint32_t) \
    (((phy_adr)->sw_head_blk  << (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) |    \
     ((phy_adr)->sw_head_page << (BKCH_SHIFT + FRAG_SHIFT)) |                 \
     ((phy_adr)->sb_head_cech << FRAG_SHIFT) |                                \
      (phy_adr)->sb_head_frag)

#define get_p4k(blk, page, bank, ch, frag) (uint32_t) \
    (((blk)  << (PAGE_SHIFT + BANK_SHIFT + CHAN_SHIFT + FRAG_SHIFT)) |   \
     ((page) << (BANK_SHIFT + CHAN_SHIFT + FRAG_SHIFT)) |                \
     ((bank) << (CHAN_SHIFT + FRAG_SHIFT)) |        \
     ((ch)   << FRAG_SHIFT) |                       \
      (frag))

#define get_blk(p4k) (uint16_t) \
    (((p4k) >> (PAGE_SHIFT + BKCH_SHIFT + FRAG_SHIFT)) & BLK_MASK)

#define get_frag(p4k) (uint8_t) \
    ((p4k) & FRAG_MASK)

#define get_plane(p4k) (uint8_t) \
    (((p4k) >> (FRAG_SHIFT - PLANE_SHIFT)) & PLANE_MASK)

#define get_lun(p4k) (uint8_t) \
    (((p4k) >> (CE_CH_FRAG_SHIFT)) & LUN_MASK)

#define get_bank(p4k) (uint8_t) \
    (((p4k) >> (CH_FRAG_SHIFT)) & BANK_MASK)

#define get_bank_of_die(die_num) (uint8_t) \
    (((die_num) >> CHAN_SHIFT) & BANK_MASK)

#define get_ce(p4k) (uint8_t) \
    (((p4k) >> (CHAN_SHIFT + FRAG_SHIFT)) & PCE_MASK)

#define get_page(p4k) (uint16_t) \
    (((p4k) >> (BKCH_SHIFT + FRAG_SHIFT)) & PAGE_MASK)

#define get_channel(p4k) (uint8_t) \
    (((p4k) >> FRAG_SHIFT) & CHAN_MASK)

#define get_channel_of_die(die_num) (uint8_t) \
    ((die_num) & CHAN_MASK)

#endif
