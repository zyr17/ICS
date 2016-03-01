#include "cache.h"
struct{
    bool valid_bit;
    uint32_t tag;
    uint8_t data[BLOCK_SIZE / 8];
}l1_cache_block[L1_SET][L1_LENGTH];

struct{
    bool valid_bit, dirty_bit;
    uint32_t tag;
    uint8_t data[BLOCK_SIZE / 8];
}l2_cache_block[L2_SET][L2_LENGTH];

uint32_t L1_cache_single(hwaddr_t addr, size_t len){
    int group = addr / BLOCK_SIZE % L1_SET;
    int tag = addr / BLOCK_SIZE / L1_SET;
    int start = addr % BLOCK_SIZE / 8;
    int pos = - 1;
    int i, j;
    for (i = 0; i < L1_LENGTH; i ++ )
        if (tag == l1_cache_block[group][i].tag)
            pos = i;
    if (!~pos){
        pos = rand() % L1_LENGTH;
        for (i = 0; i < len; i ++ )
            l1_cache_block[group][pos].data[i + start] = dram_read(addr + i, 1);
    }
    uint32_t ans = 0;
    for (j = 0; j < len; j ++ )
        ans = (ans << 8) + l1_cache_block[group][pos].data[j + start];
    return ans;
}

uint32_t L1_cache_read(hwaddr_t addr,size_t len){
    if (addr / BLOCK_SIZE != (addr + len - 1) / BLOCK_SIZE){
        int tmp = (addr + len - 1) % BLOCK_SIZE + 1;
        uint32_t t1 = L1_cache_single(addr, len - tmp);
        t1 <<= (tmp * 8);
        t1 += L1_cache_single(addr + (len - tmp) * 8, tmp);
        return t1;
    }
    else return L1_cache_single(addr, len);
}
