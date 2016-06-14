#include "common.h"
#include "memory/cache.h"
#include "memory/page.h"
#include "cpu/reg.h"
#include "device/mmio.h"

extern uint8_t *hw_mem;

inline uint32_t dram_read(hwaddr_t, size_t);
inline void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

inline uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	uint32_t read;
	uint32_t pid = is_mmio(addr);
	if (pid != -1) return mmio_read(addr, len, pid);
	#ifdef USE_CACHE
	read = L1_cache_read(addr, len) & (~0u >> ((4 - len) << 3));
	#else
	#ifdef DIRECTLY_MEM
    read = (*(uint32_t*)(hw_mem + addr)) & (~0u >> ((4 - len) << 3));
	#else
	read = dram_read(addr, len) & (~0u >> ((4 - len) << 3));
	#endif
	#endif
	/*uint32_t re2 = dram_read(addr, len) & (~0u >> ((4 - len) << 3));
	if (read != re2){
        printf("\n%d %d: ", (int)addr, (int)len);
        printf("%x %x\n", (int)read, (int)re2);
    }*/
    return read;
}

inline void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	uint32_t pid = is_mmio(addr);
	if (pid != -1){
        //Log("mmio_get, 0x%x", addr);
        mmio_write(addr, len, data, pid);
        return;
    }
    #ifdef USE_CACHE
	L1_cache_write(addr, len, data);
	#else
	#ifdef DIRECTLY_MEM
	assert(len == 1 || len == 2 || len == 4);
    if (len == 1) *(uint8_t*)(hw_mem + addr) = data & (~0u >> ((4 - len) << 3));
    else if (len == 2) *(uint16_t*)(hw_mem + addr) = data & (~0u >> ((4 - len) << 3));
    else if (len == 4) *(uint32_t*)(hw_mem + addr) = data & (~0u >> ((4 - len) << 3));
    else panic("len error");
	#else
	dram_write(addr, len, data);
	#endif
	#endif
}

inline uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
    #ifdef DEBUG
    assert(len == 1 || len == 2 || len == 4);
    #endif
	return page_read(addr, len);
}

inline void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
    #ifdef DEBUG
    assert(len == 1 || len == 2 || len == 4);
    #endif
	page_write(addr, len, data);
}

inline lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg_num){
    if (!cpu.PE) return addr;
    assert(cpu.sreg_limit[sreg_num] >= (cpu.sreg_base[sreg_num] & 0xfffffff8) + addr + len - 1);
    return addr + (cpu.sreg_base[sreg_num] & 0xfffffff8);
}

inline uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
    #ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
	#endif
    lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	return lnaddr_read(lnaddr, len);
}

inline void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
    #ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
	#endif
    lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	lnaddr_write(lnaddr, len, data);
}

/*
 *
 *
 * page.c is copied below for inline to boost
 *
 *
 *
 */





#include "memory/page.h"

hwaddr_t __attribute__((noinline)) page_translate_real(lnaddr_t addr){
    hwaddr_t res = 0, tmp;
    tmp = cpu.cr3 + (addr >> 22) * 4;
    uint32_t tval = hwaddr_read(tmp, 4);
    Assert(tval & 1, "eip:%x, %x %x %x", cpu.eip, addr, tmp, tval);
    tval = hwaddr_read((tval & 0xfffff000) + ((addr >> 12) & 0x3ff) * 4, 4);
    Assert(tval & 1, "eip:%x, %x %x", cpu.eip, addr, tval);
    res = (tval & 0xfffff000) + (addr & 0xfff);
    return res;
}

inline hwaddr_t page_translate(lnaddr_t addr){
#ifdef USE_VERY_BIG_TLB
    if (bbtlb[addr >> 12] & 1){
        bbtlb[addr >> 12] = (page_translate_real(addr) & 0xfffff000);
    }
    return (addr & 0xfff) + bbtlb[addr >> 12];
#else
    int i;
    for (i = 0; i < TLB_SIZE; i ++ )
        if (TLB_cache[i].valid_bit && addr >> 12 == TLB_cache[i].tag){
            tmp_TLB_cache = TLB_cache[i];
            int j;
            for (j = i; j; j -- )
                TLB_cache[j] = TLB_cache[j - 1];
            TLB_cache[0] = tmp_TLB_cache;
            return (TLB_cache[0].data << 12) + (addr & 0xfff);
        }
    hwaddr_t res = page_translate_real(addr);
    int pos = rand() % TLB_SIZE;
    TLB_cache[pos].valid_bit = 1;
    TLB_cache[pos].tag = addr >> 12;
    TLB_cache[pos].data = res >> 12;
    return res;
#endif
}

uint32_t page_read(lnaddr_t addr, size_t len){
    if (!cpu.PG) return hwaddr_read(addr, len);
    if (addr / PAGE_SIZE != (addr + len - 1) / PAGE_SIZE){
        int tmp = (addr + len - 1) % PAGE_SIZE + 1;
        uint32_t t1 = hwaddr_read(page_translate(addr + (len - tmp)), tmp);
        t1 <<= ((len - tmp) * 8);
        t1 += hwaddr_read(page_translate(addr), len - tmp);
        return t1;
    }
    else return hwaddr_read(page_translate(addr), len);
}

void page_write(lnaddr_t addr, size_t len, uint32_t data){
    if (!cpu.PG){
        hwaddr_write(addr, len, data);
        return;
    }
    if (addr / PAGE_SIZE != (addr + len - 1) / PAGE_SIZE){
        int tmp = (addr + len - 1) % PAGE_SIZE + 1;
        hwaddr_write(page_translate(addr + (len - tmp)), tmp, data >> ((len - tmp) * 8));
        hwaddr_write(page_translate(addr), len - tmp, data & ((1 << ((len - tmp) * 8)) - 1));
    }
    else hwaddr_write(page_translate(addr), len, data);
}

void page_find_real(lnaddr_t addr){
    int i;
    for (i = 0; i < TLB_SIZE; i ++ )
        if (TLB_cache[i].valid_bit && addr >> 12 == TLB_cache[i].tag)
            printf("find in TLB, translated addr: 0x%08x\n", (TLB_cache[0].data << 12) + (addr & 0xfff));

    hwaddr_t tmp;
    tmp = cpu.cr3 + (addr >> 22) * 4;
    uint32_t tval = hwaddr_read(tmp, 4);

    if (tval & 1){
        printf("find page v1, input: 0x%03x, page table addr: 0x%08x\n", addr >> 22, tval);
        tval = hwaddr_read((tval & 0xfffff000) + ((addr >> 12) & 0x3ff) * 4, 4);
        if (tval & 1){
            printf("find page v2, translated addr: 0x%08x\n", (tval & 0xfffff000) + (addr & 0xfff));
        }
        else{
            printf("not find page v2\n");
        }
    }
    else{
        printf("not find page v1\n");
    }
}

void page_check(lnaddr_t addr){
    printf("input ADDR: 0x%08x\n", addr);
#ifdef USE_VERY_BIG_TLB
    printf("now using VERY BIG TLB to boost!\n");printf("%d\n", bbtlb[addr >> 12]);
    if (!(bbtlb[addr >> 12] & 1)) printf("find in VERY BIG TLB, translated addr: 0x%08x\n", (addr & 0xfff) + bbtlb[addr >> 12]);
    else{
        printf("not find in VERY BIG TLB.\n");
#endif
    page_find_real(addr);
#ifdef USE_VERY_BIG_TLB
    }
#endif
}
