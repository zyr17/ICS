#include "memory/cache.h"

#define USE_L2_CACHE

void init_cache(){
    memset(l1_cache_block, 0, sizeof l1_cache_block);
    memset(l2_cache_block, 0, sizeof l2_cache_block);
}

inline void L2_bubble(int k1, int k2){
    l2_cache_temp = l2_cache_block[k1][k2];
    int i;
    for (i = k2; i; i -- )
        l2_cache_block[k1][i] = l2_cache_block[k1][i - 1];
    l2_cache_block[k1][0] = l2_cache_temp;
}

inline uint32_t L2_cache_single(hwaddr_t addr, size_t len){
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
            //int ii;
            //for (ii = BLOCK_SIZE / 8 - 1; ii >= 0; ii -- )
            //    lltmp = (lltmp << 8LL) + l2_cache_block[group][pos].data[ii];
            //dram_write(addr_old, 4, lltmp & 0xffffffff);
            //dram_write(addr_old + 4, 4, lltmp >> 32LL);
            //printf("%08llX %08X\n", lltmp & 0xffffffff, l2_cache_block[group][pos].data_32_low);
            dram_write(addr_old, 4, l2_cache_block[group][pos].data_32_low);
            dram_write(addr_old + 4, 4, l2_cache_block[group][pos].data_32_high);
            //int tlow = dram_read(addr_old, 4);
            //int thigh = dram_read(addr_old + 4, 4);
            //if (tlow != l2_cache_block[group][pos].data_32_low) printf("%x %x\n", tlow, l2_cache_block[group][pos].data_32_low);
            //if (thigh != l2_cache_block[group][pos].data_32_high) printf("%x %x\n", thigh, l2_cache_block[group][pos].data_32_high);
            /*printf("%x %x, ", tlow, thigh);
            int q;
            for (q = 0; q < 8; q ++ )
                printf("%2x ", l2_cache_block[group][pos].data[q]);
            printf("\n");*/
            //int ii;
            //for (ii = 0; ii < BLOCK_SIZE / 8; ii ++ )
            //    dram_write(addr_old + ii, 1, l2_cache_block[group][pos].data[ii]);
        }
        l2_cache_block[group][pos].valid_bit = 1;
        l2_cache_block[group][pos].tag = tag;
        /*for (i = 0; i < BLOCK_SIZE / 8; i ++ )
            l2_cache_block[group][pos].data[i] = dram_read(addr / (BLOCK_SIZE / 8) * (BLOCK_SIZE / 8) + i, 1);*/
        hwaddr_t addr_old = addr / (BLOCK_SIZE / 8) * (BLOCK_SIZE / 8);
        l2_cache_block[group][pos].data_32_low = dram_read(addr_old, 4);
        l2_cache_block[group][pos].data_32_high = dram_read(addr_old + 4, 4);
    }
    uint32_t ans = 0;
    for (j = len - 1; j >= 0; j -- )
        ans = (ans << 8) + l2_cache_block[group][pos].data[j + start];
    L2_bubble(group, pos);
    return ans;
}

inline uint32_t L2_cache_read(hwaddr_t addr, size_t len){
    if (addr / (BLOCK_SIZE / 8) != (addr + len - 1) / (BLOCK_SIZE / 8)){
        int tmp = (addr + len - 1) % (BLOCK_SIZE / 8) + 1;
        uint32_t t1 = L2_cache_single(addr + (len - tmp), tmp);
        t1 <<= ((len - tmp) * 8);
        t1 += L2_cache_single(addr, len - tmp);
        return t1;
    }
    else return L2_cache_single(addr, len);
}

inline void L2_cache_update(hwaddr_t addr, size_t len, uint32_t data){
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
            /*int ii;
            for (ii = 0; ii < BLOCK_SIZE / 8; ii ++ )
                dram_write(addr_old + ii, 1, l2_cache_block[group][pos].data[ii]);*/
            dram_write(addr_old, 4, l2_cache_block[group][pos].data_32_low);
            dram_write(addr_old + 4, 4, l2_cache_block[group][pos].data_32_high);
        }
        l2_cache_block[group][pos].valid_bit = 1;
        l2_cache_block[group][pos].tag = tag;
        hwaddr_t old_addr = addr / (BLOCK_SIZE / 8) * (BLOCK_SIZE / 8);
        l2_cache_block[group][pos].data_32_low = dram_read(old_addr, 4);
        l2_cache_block[group][pos].data_32_high = dram_read(old_addr + 4, 4);
        /*unsigned long long lltmp = ((unsigned long long)(dram_read(old_addr + 4, 4)) << 32LL) + dram_read(old_addr, 4);
        for (i = 0; i < BLOCK_SIZE / 8; i ++ ){
            l2_cache_block[group][pos].data[i] = lltmp & 0xff;
            lltmp >>= 8;
            //l2_cache_block[group][pos].data[i] = dram_read(addr / (BLOCK_SIZE / 8) * (BLOCK_SIZE / 8) + i, 1);
        }*/
    }
    l2_cache_block[group][pos].dirty_bit = 1;
    for (ii = 0; ii < len; ii ++ ){
        l2_cache_block[group][pos].data[ii + start] = data & 0xff;
        data >>= 8;
    }
    L2_bubble(group, pos);
}

inline void L2_cache_write(hwaddr_t addr, size_t len, uint32_t data){
    if (addr / (BLOCK_SIZE / 8) != (addr + len - 1) / (BLOCK_SIZE / 8)){
        int tmp = (addr + len - 1) % (BLOCK_SIZE / 8) + 1;
        L2_cache_update(addr + (len - tmp), tmp, data >> ((len - tmp) * 8));
        L2_cache_update(addr, len - tmp, data & ((1 << ((len - tmp) * 8)) - 1));
    }
    else L2_cache_update(addr, len, data);
}

inline void L1_bubble(int k1, int k2){
    l1_cache_temp = l1_cache_block[k1][k2];
    int i;
    for (i = k2; i; i -- )
        l1_cache_block[k1][i] = l1_cache_block[k1][i - 1];
    l1_cache_block[k1][0] = l1_cache_temp;
}

inline uint32_t L1_cache_single(hwaddr_t addr, size_t len){
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
		#ifdef USE_L2_CACHE
        //l1_cache_block[group][pos].data_32_low = L2_cache_read(old_addr, 4);
        //l1_cache_block[group][pos].data_32_high = L2_cache_read(old_addr + 4, 4);
        for (i = 0; i < BLOCK_SIZE / 8 / 4; i ++ )
            l1_cache_block[group][pos].data_32[i] = L2_cache_read(old_addr + i * 4, 4);
		#else
        l1_cache_block[group][pos].data_32_low = dram_read(old_addr, 4);
        l1_cache_block[group][pos].data_32_high = dram_read(old_addr + 4, 4);
		#endif
        /*unsigned long long lltmp = ((unsigned long long)L2_cache_read(old_addr + 4, 4) << 32) + L2_cache_read(old_addr, 4);
        for (i = 0; i < BLOCK_SIZE / 8; i ++ ){
            l1_cache_block[group][pos].data[i] = lltmp & 0xff;
            lltmp >>= 8;
            //l1_cache_block[group][pos].data[i] = L2_cache_read(addr / (BLOCK_SIZE / 8) * (BLOCK_SIZE / 8) + i, 1);
        }*/
    }
    uint32_t ans = 0;
    for (j = len - 1; j >= 0; j -- )
        ans = (ans << 8) + l1_cache_block[group][pos].data[j + start];
    L1_bubble(group, pos);
    return ans;
}

inline uint32_t L1_cache_read(hwaddr_t addr, size_t len){
    if (addr / (BLOCK_SIZE / 8) != (addr + len - 1) / (BLOCK_SIZE / 8)){
        int tmp = (addr + len - 1) % (BLOCK_SIZE / 8) + 1;
        uint32_t t1 = L1_cache_single(addr + (len - tmp), tmp);
        t1 <<= ((len - tmp) * 8);
        t1 += L1_cache_single(addr, len - tmp);
        return t1;
    }
    else return L1_cache_single(addr, len);
}

inline void L1_cache_update(hwaddr_t addr, size_t len){
    int group = addr / (BLOCK_SIZE / 8) % L1_SET;
    int tag = addr / (BLOCK_SIZE / 8) / L1_SET;
    int start = addr % (BLOCK_SIZE / 8);
    int i, ii;
    for (i = 0; i < L1_LENGTH; i ++ )
        if (l1_cache_block[group][i].valid_bit == 1 && tag == l1_cache_block[group][i].tag){
			#ifdef USE_L2_CACHE
            uint32_t ltmp = L2_cache_read(addr, len);
			#else
            uint32_t ltmp = dram_read(addr, len);
			#endif
            for (ii = 0; ii < len; ii ++ ){
                l1_cache_block[group][i].data[ii + start] = ltmp & 0xff;
                ltmp >>= 8;
                //l1_cache_block[group][i].data[ii + start] = L2_cache_read(addr + ii, 1);
            }
            L1_bubble(group, i);
        }
}

inline void L1_cache_write(hwaddr_t addr, size_t len, uint32_t data){
	#ifdef USE_L2_CACHE
    L2_cache_write(addr, len, data);
	#else
    dram_write(addr, len, data);
	#endif
    if (addr / (BLOCK_SIZE / 8) != (addr + len - 1) / (BLOCK_SIZE / 8)){
        int tmp = (addr + len - 1) % (BLOCK_SIZE / 8) + 1;
        L1_cache_update(addr + (len - tmp), tmp);
        L1_cache_update(addr, len - tmp);
    }
    else L1_cache_update(addr, len);
}

inline void L2_cache_check(hwaddr_t addr){
    int group = addr / (BLOCK_SIZE / 8) % L2_SET;
    int tag = addr / (BLOCK_SIZE / 8) / L2_SET;
    int i;
    for (i = 0; i < L2_LENGTH; i ++ )
        if (l2_cache_block[group][i].valid_bit && l2_cache_block[group][i].tag == tag){
            printf("L2 cache: find addr: 0x%x, group: 0x%x, tag: 0x%x, position: %d\n, dirty bit: %d\nblock content:\n", addr, group, tag, i, l2_cache_block[group][i].dirty_bit);
            int j;
            for (j = 0; j < BLOCK_SIZE / 8; j ++ ){
                printf("%02x ", l2_cache_block[group][i].data[j]);
                if ((j + 1) % 8 == 0) printf("\n");
            }
            return;
        }
    printf("L2 cache: not find addr: 0x%x\n", addr);
}

inline void L1_cache_check(hwaddr_t addr){
    int group = addr / (BLOCK_SIZE / 8) % L1_SET;
    int tag = addr / (BLOCK_SIZE / 8) / L1_SET;
    int i;
    for (i = 0; i < L1_LENGTH; i ++ )
        if (l1_cache_block[group][i].valid_bit && l1_cache_block[group][i].tag == tag){
            printf("L1 cache: find addr: 0x%x, group: 0x%x, tag: 0x%x, position: %d\nblock content:\n", addr, group, tag, i);
            int j;
            for (j = 0; j < BLOCK_SIZE / 8; j ++ ){
                printf("%02x ", l1_cache_block[group][i].data[j]);
                if ((j + 1) % 8 == 0) printf("\n");
            }
#ifdef USE_L2_CACHE
            L2_cache_check(addr);
#else
            printf("L2 cache not open!\n");
#endif
            return;
        }
    printf("L1 cache: not find addr: 0x%x\n", addr);
#ifdef USE_L2_CACHE
    L2_cache_check(addr);
#else
    printf("L2 cache not open!\n");
#endif
}

inline void cache_check(hwaddr_t addr){
    printf("Block ADDR: 0x%x\n", addr &= ~((1 << BLOCK_BITS) - 1));
#ifdef USE_CACHE
    L1_cache_check(addr);
#else
    printf("cache not open!\n");
#endif
}
