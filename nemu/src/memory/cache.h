#include "common.h"
#include <stdlib.h>
#define BLOCK_SIZE 64
#define L1_SIZE 65536
#define L1_SET 8
#define L1_LENGTH L1_SIZE / L1_SET / BLOCK_SIZE
#define L2_SIZE 4194304
#define L2_SET 16
#define L2_LENGTH L2_SIZE / L2_SET / BLOCK_SIZE
uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);
inline uint32_t L1_cache_read(hwaddr_t addr,size_t len);
inline void L1_cache_write(hwaddr_t addr, size_t len, uint32_t data);
inline uint32_t L2_cache_read(hwaddr_t addr,size_t len);
inline void L2_cache_write(hwaddr_t addr, size_t len, uint32_t data);
