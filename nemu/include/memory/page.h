#ifndef __PAGE_H__
#define __PAGE_H__

#include "common.h"
#include "cpu/reg.h"
#include "memory/memory.h"
#include <stdlib.h>

#define PAGE_SIZE 4096
#define TLB_SIZE 64

struct{
    bool valid_bit;
    uint32_t data:20;
    uint32_t tag:20;
}TLB_cache[TLB_SIZE], tmp_TLB_cache;

uint32_t page_read(lnaddr_t, size_t);
void page_write(lnaddr_t, size_t, uint32_t);

#ifdef USE_VERY_BIG_TLB
uint32_t bbtlb[1111111];
#endif

#endif
