#include "cpu/exec/helper.h"

#if DATA_BYTE == 1

#define SUFFIX b
#define DATA_TYPE uint8_t
#define DATA_TYPE_S int8_t

#elif DATA_BYTE == 2

#define SUFFIX w
#define DATA_TYPE uint16_t
#define DATA_TYPE_S int16_t

#elif DATA_BYTE == 4

#define SUFFIX l
#define DATA_TYPE uint32_t
#define DATA_TYPE_S int32_t

#else

#error unknown DATA_BYTE

#endif

#define REG(index) concat(reg_, SUFFIX) (index)
#define REG_NAME(index) concat(regs, SUFFIX) [index]

#define MEM_R(addr) swaddr_read(addr, DATA_BYTE)
#define MEM_W(addr, data) swaddr_write(addr, DATA_BYTE, data)

#define OPERAND_W(op, src) concat(write_operand_, SUFFIX) (op, src)

#define MSB(n) ((DATA_TYPE)(n) >> ((DATA_BYTE << 3) - 1))

//#ifndef __EFLAGS_ADD_SUB__
//#define __EFLAGS_ADD_SUB__

//static void eflags_add_sub(uint32_t dest, uint32_t src, uint32_t sub, uint32_t byte){

#define eflags_add_sub(_dest, _src, _sub, _byte)\
do {\
    uint32_t dest = (_dest), src = (_src), sub = (_sub), byte = (_byte);\
    byte *= 8;\
    uint32_t y = sub ? ~ src : src;\
    uint32_t res = dest + y + sub;\
    res &= ((1 << (byte - 1)) << 1) - 1;\
    dest &= ((1 << (byte - 1)) << 1) - 1;\
    src &= ((1 << (byte - 1)) << 1) - 1;\
    y &= ((1 << (byte - 1)) << 1) - 1;\
    cpu.ZF = !res;\
    cpu.OF = (dest & (1 << (byte - 1))) == (y  & (1 << (byte - 1))) && (y  & (1 << (byte - 1))) != (res  & (1 << (byte - 1)));\
    cpu.CF = sub ^ (res < dest || res < y);\
    cpu.PF = 1;\
    uint32_t i = 1, j = res;\
    for (; i <= 8; i ++ , j /= 2)\
        cpu.PF ^= j & 1;\
    cpu.SF = res  >> (byte - 1);\
}while (0)

#define push_push(_byte, _data) MEM_W(cpu.esp -= (_byte), (_data))

#define pop_pop(_byte, _data)\
do{\
    _data = MEM_R(cpu.esp);\
    cpu.esp += _byte;\
}while (0)


//#endif

