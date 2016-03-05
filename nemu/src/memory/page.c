#include "page.h"

hwaddr_t page_translate(lnaddr_t addr){
    hwaddr_t res = 0, tmp;
    tmp = cpu.cr3 + (addr >> 22) * 4;
    uint32_t tval = hwaddr_read(tmp, 4);
    assert(tval >> 31);
    tval = hwaddr_read((tval << 12) + ((addr >> 12) & 0x3ff) * 4, 4);
    assert(tval >> 31);
    res = (tval << 12) + (addr & 0xfff);
    return res;
}

uint32_t page_read(lnaddr_t addr, size_t len){
    if (!cpu.PG) hwaddr_read(addr, len);
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
    if (!cpu.PG) hwaddr_write(addr, len, data);
    if (addr / PAGE_SIZE != (addr + len - 1) / PAGE_SIZE){
        int tmp = (addr + len - 1) % PAGE_SIZE + 1;
        hwaddr_write(page_translate(addr + (len - tmp)), tmp, data & ((1 << (tmp * 8)) - 1));
        hwaddr_write(page_translate(addr), len - tmp, data >> (tmp * 8));
    }
    else hwaddr_write(page_translate(addr), len, data);
}
