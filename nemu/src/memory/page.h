#ifndef __PAGE_H__
#define __PAGE_H__

#include "common.h"
#include "cpu/reg.h"
#include "memory/memory.h"

#define PAGE_SIZE 4096

uint32_t page_read(lnaddr_t, size_t);
void page_write(lnaddr_t, size_t, uint32_t);

#endif
