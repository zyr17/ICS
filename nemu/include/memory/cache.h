#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"
#include <stdlib.h>

#define BLOCK_BITS 6
#define BLOCK_SIZE (8 << BLOCK_BITS)
#define L1_SIZE (65536 * 8)
#define L1_SET 8
#define L1_LENGTH (L1_SIZE / L1_SET / BLOCK_SIZE)
#define L2_SIZE (4194304 * 8)
#define L2_SET 16
#define L2_LENGTH (L2_SIZE / L2_SET / BLOCK_SIZE)

struct{
    bool valid_bit;
    uint32_t tag;
    union{
        uint8_t data[BLOCK_SIZE / 8];
        uint32_t data_32[BLOCK_SIZE / 8 / 4];
    };
}l1_cache_block[L1_SET][L1_LENGTH], l1_cache_temp;

struct{
    bool valid_bit, dirty_bit;
    uint32_t tag;
    union{
        uint8_t data[BLOCK_SIZE / 8];
        uint32_t data_32[BLOCK_SIZE / 8 / 4];
    };
}l2_cache_block[L2_SET][L2_LENGTH], l2_cache_temp;

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);
inline uint32_t L1_cache_read(hwaddr_t addr,size_t len);
inline void L1_cache_write(hwaddr_t addr, size_t len, uint32_t data);
inline uint32_t L2_cache_read(hwaddr_t addr,size_t len);
inline void L2_cache_write(hwaddr_t addr, size_t len, uint32_t data);
inline void cache_check(hwaddr_t addr);

#endif
