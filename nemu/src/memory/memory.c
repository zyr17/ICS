#include "common.h"
#include "cache.h"
#include "page.h"
#include "cpu/reg.h"

inline uint32_t dram_read(hwaddr_t, size_t);
inline void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

inline uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	uint32_t read;
	#ifdef USE_CACHE
	read = L1_cache_read(addr, len) & (~0u >> ((4 - len) << 3));
	#else
	read = dram_read(addr, len) & (~0u >> ((4 - len) << 3));
	#endif
	/*uint32_t re2 = dram_read(addr, len) & (~0u >> ((4 - len) << 3));
	if (read != re2){
        printf("\n%d %d: ", (int)addr, (int)len);
        printf("%x %x\n", (int)read, (int)re2);
    }*/
    return read;
}

inline void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
    #ifdef USE_CACHE
	L1_cache_write(addr, len, data);
	#else
	dram_write(addr, len, data);
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
    assert(cpu.sreg_limit[sreg_num] >= cpu.sreg_base[sreg_num] + addr + len - 1);
    return addr + cpu.sreg_base[sreg_num];
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

