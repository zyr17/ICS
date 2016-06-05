#include "memory/page.h"

hwaddr_t page_translate_real(lnaddr_t addr){
    hwaddr_t res = 0, tmp;
    tmp = cpu.cr3 + (addr >> 22) * 4;
    uint32_t tval = hwaddr_read(tmp, 4);
    assert(tval & 1);
    tval = hwaddr_read((tval & 0xfffff000) + ((addr >> 12) & 0x3ff) * 4, 4);
    assert(tval & 1);
    res = (tval & 0xfffff000) + (addr & 0xfff);
    return res;
}

hwaddr_t page_translate(lnaddr_t addr){
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
        hwaddr_write(page_translate(addr + (len - tmp)), tmp, data & ((1 << (tmp * 8)) - 1));
        hwaddr_write(page_translate(addr), len - tmp, data >> (tmp * 8));
    }
    else hwaddr_write(page_translate(addr), len, data);
}
