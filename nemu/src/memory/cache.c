#include "cache.h"
struct{
    bool valid_bit;
    uint32_t tag;
    union{
        uint8_t data[BLOCK_SIZE / 8];
        volatile uint32_t data_32_low, data_32_high;
    };
}l1_cache_block[L1_SET][L1_LENGTH], l1_cache_temp;

struct{
    bool valid_bit, dirty_bit;
    uint32_t tag;
    union{
        uint8_t data[BLOCK_SIZE / 8];
        volatile uint32_t data_32_low, data_32_high;
    };
}l2_cache_block[L2_SET][L2_LENGTH], l2_cache_temp;

void L2_bubble(int k1, int k2){
    l2_cache_temp = l2_cache_block[k1][k2];
    int i;
    for (i = k2; i; i -- )
        l2_cache_block[k1][i] = l2_cache_block[k1][i - 1];
    l2_cache_block[k1][0] = l2_cache_temp;
}

uint32_t L2_cache_single(hwaddr_t addr, size_t len){
    int group = addr / (BLOCK_SIZE / 8) % L2_SET;
    int tag = addr / (BLOCK_SIZE / 8) / L2_SET;
    int start = addr % (BLOCK_SIZE / 8);
    int pos = - 1;
    int i, j;
    for (i = 0; i < L2_LENGTH; i ++ )
        if (l2_cache_block[group][i].valid_bit == 1 && tag == l2_cache_block[group][i].tag)
            pos = i;
    if (!~pos){
        pos = rand() % L2_LENGTH;
        if (l2_cache_block[group][pos].valid_bit && l2_cache_block[group][pos].dirty_bit){
            l2_cache_block[group][pos].dirty_bit = 0;
            hwaddr_t addr_old = (l2_cache_block[group][pos].tag * L2_SET + group) * (BLOCK_SIZE / 8);
            //unsigned long long lltmp = 0;
            //for (ii = BLOCK_SIZE / 8 - 1; i >= 0; i -- )
            //    lltmp = (lltmp << 8LL) + l2_cache_block[group][pos].data[ii];
            dram_write(addr_old, 4, l2_cache_block[group][pos].data_32_low);
            dram_write(addr_old + 4, 4, l2_cache_block[group][pos].data_32_high);
            unsigned ii;
            for (ii = 0; ii < BLOCK_SIZE / 8; ii ++ )
                dram_write(addr_old + ii, 1, l2_cache_block[group][pos].data[ii]);
        }
        l2_cache_block[group][pos].valid_bit = 1;
        l2_cache_block[group][pos].tag = tag;
        for (i = 0; i < BLOCK_SIZE / 8; i ++ )
            l2_cache_block[group][pos].data[i] = dram_read(addr / (BLOCK_SIZE / 8) * (BLOCK_SIZE / 8) + i, 1);
    }
    uint32_t ans = 0;
    for (j = len - 1; j >= 0; j -- )
        ans = (ans << 8) + l2_cache_block[group][pos].data[j + start];
    L2_bubble(group, pos);
    return ans;
}

uint32_t L2_cache_read(hwaddr_t addr, size_t len){
    if (addr / (BLOCK_SIZE / 8) != (addr + len - 1) / (BLOCK_SIZE / 8)){
        int tmp = (addr + len - 1) % (BLOCK_SIZE / 8) + 1;
        uint32_t t1 = L2_cache_single(addr + (len - tmp), tmp);
        t1 <<= ((len - tmp) * 8);
        t1 += L2_cache_single(addr, len - tmp);
        return t1;
    }
    else return L2_cache_single(addr, len);
}

void L2_cache_update(hwaddr_t addr, size_t len, uint32_t data){
    int group = addr / (BLOCK_SIZE / 8) % L2_SET;
    int tag = addr / (BLOCK_SIZE / 8) / L2_SET;
    int start = addr % (BLOCK_SIZE / 8);
    int i, ii;
    int pos = - 1;
    for (i = 0; i < L2_LENGTH; i ++ )
        if (l2_cache_block[group][i].valid_bit == 1 && tag == l2_cache_block[group][i].tag)
            pos = i;
    if (!~pos){
        pos = rand() % L2_LENGTH;
        if (l2_cache_block[group][pos].dirty_bit){
            l2_cache_block[group][pos].dirty_bit = 0;
            hwaddr_t addr_old = (l2_cache_block[group][pos].tag * L2_SET + group) * (BLOCK_SIZE / 8);
            int ii;
            for (ii = 0; ii < BLOCK_SIZE / 8; ii ++ )
                dram_write(addr_old + ii, 1, l2_cache_block[group][pos].data[ii]);
        }
        l2_cache_block[group][pos].valid_bit = 1;
        l2_cache_block[group][pos].tag = tag;
        hwaddr_t old_addr = addr / (BLOCK_SIZE / 8) * (BLOCK_SIZE / 8);
        unsigned long long lltmp = ((unsigned long long)(dram_read(old_addr + 4, 4)) << 32LL) + dram_read(old_addr, 4);
        for (i = 0; i < BLOCK_SIZE / 8; i ++ ){
            l2_cache_block[group][pos].data[i] = lltmp & 0xff;
            lltmp >>= 8;
            //l2_cache_block[group][pos].data[i] = dram_read(addr / (BLOCK_SIZE / 8) * (BLOCK_SIZE / 8) + i, 1);
        }
    }
    l2_cache_block[group][pos].dirty_bit = 1;
    for (ii = 0; ii < len; ii ++ ){
        l2_cache_block[group][pos].data[ii + start] = data & 0xff;
        data >>= 8;
    }
    L2_bubble(group, pos);
}

void L2_cache_write(hwaddr_t addr, size_t len, uint32_t data){
    if (addr / (BLOCK_SIZE / 8) != (addr + len - 1) / (BLOCK_SIZE / 8)){
        int tmp = (addr + len - 1) % (BLOCK_SIZE / 8) + 1;
        L2_cache_update(addr + (len - tmp), tmp, data & ((1 << (tmp * 8)) - 1));
        L2_cache_update(addr, len - tmp, data >> (tmp * 8));
    }
    else L2_cache_update(addr, len, data);
}

void L1_bubble(int k1, int k2){
    l1_cache_temp = l1_cache_block[k1][k2];
    int i;
    for (i = k2; i; i -- )
        l1_cache_block[k1][i] = l1_cache_block[k1][i - 1];
    l1_cache_block[k1][0] = l1_cache_temp;
}

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
        hwaddr_t old_addr = addr / (BLOCK_SIZE / 8) * (BLOCK_SIZE / 8);
        unsigned long long lltmp = ((unsigned long long)L2_cache_read(old_addr + 4, 4) << 32) + L2_cache_read(old_addr, 4);
        for (i = 0; i < BLOCK_SIZE / 8; i ++ ){
            l1_cache_block[group][pos].data[i] = lltmp & 0xff;
            lltmp >>= 8;
            //l1_cache_block[group][pos].data[i] = L2_cache_read(addr / (BLOCK_SIZE / 8) * (BLOCK_SIZE / 8) + i, 1);
        }
    }
    uint32_t ans = 0;
    for (j = len - 1; j >= 0; j -- )
        ans = (ans << 8) + l1_cache_block[group][pos].data[j + start];
    L1_bubble(group, pos);
    return ans;
}

uint32_t L1_cache_read(hwaddr_t addr, size_t len){
    if (addr / (BLOCK_SIZE / 8) != (addr + len - 1) / (BLOCK_SIZE / 8)){
        int tmp = (addr + len - 1) % (BLOCK_SIZE / 8) + 1;
        uint32_t t1 = L1_cache_single(addr + (len - tmp), tmp);
        t1 <<= ((len - tmp) * 8);
        t1 += L1_cache_single(addr, len - tmp);
        return t1;
    }
    else return L1_cache_single(addr, len);
}

void L1_cache_update(hwaddr_t addr, size_t len){
    int group = addr / (BLOCK_SIZE / 8) % L1_SET;
    int tag = addr / (BLOCK_SIZE / 8) / L1_SET;
    int start = addr % (BLOCK_SIZE / 8);
    int i, ii;
    for (i = 0; i < L1_LENGTH; i ++ )
        if (l1_cache_block[group][i].valid_bit == 1 && tag == l1_cache_block[group][i].tag){
            uint32_t ltmp = L2_cache_read(addr, len);
            for (ii = 0; ii < len; ii ++ ){
                l1_cache_block[group][i].data[ii + start] = ltmp & 0xff;
                ltmp >>= 8;
                //l1_cache_block[group][i].data[ii + start] = L2_cache_read(addr + ii, 1);
            }
            L1_bubble(group, i);
        }
}

void L1_cache_write(hwaddr_t addr, size_t len, uint32_t data){
    L2_cache_write(addr, len, data);
    if (addr / (BLOCK_SIZE / 8) != (addr + len - 1) / (BLOCK_SIZE / 8)){
        int tmp = (addr + len - 1) % (BLOCK_SIZE / 8) + 1;
        L1_cache_update(addr + (len - tmp), tmp);
        L1_cache_update(addr, len - tmp);
    }
    else L1_cache_update(addr, len);
}
