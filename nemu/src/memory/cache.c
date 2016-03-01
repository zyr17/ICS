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
    int group = addr / (BLOCK_SIZE / 8) % L1_SET;
    int tag = addr / (BLOCK_SIZE / 8) / L1_SET;
    int start = addr % (BLOCK_SIZE / 8);
    int pos = - 1;
    int i, j;
    for (i = 0; i < L1_LENGTH; i ++ )
        if (l1_cache_block[group][i].valid_bit == 1 && tag == l1_cache_block[group][i].tag)
            pos = i;
    if (!~pos){
        pos = rand() % L1_LENGTH;
        l1_cache_block[group][pos].valid_bit = 1;
        l1_cache_block[group][pos].tag = tag;
        for (i = 0; i < BLOCK_SIZE / 8; i ++ )
            l1_cache_block[group][pos].data[i] = dram_read(addr / (BLOCK_SIZE / 8) * (BLOCK_SIZE / 8) + i, 1);
    }
    uint32_t ans = 0;
    for (j = len - 1; j >= 0; j -- )
        ans = (ans << 8) + l1_cache_block[group][pos].data[j + start];
    return ans;
}

uint32_t L1_cache_read(hwaddr_t addr, size_t len){
    if (addr / (BLOCK_SIZE / 8) != (addr + len - 1) / (BLOCK_SIZE / 8)){
        int tmp = (addr + len - 1) % (BLOCK_SIZE / 8) + 1;
        uint32_t t1 = L1_cache_single(addr + (len - tmp), tmp);
        t1 <<= (tmp * 8);
        t1 += L1_cache_single(addr, len - tmp);
        return t1;
    }
    else return L1_cache_single(addr, len);
}

void L1_cache_write(hwaddr_t addr, size_t len, uint32_t data){
    dram_write(addr, len, data);
    L1_cache_read(addr, len);
}
