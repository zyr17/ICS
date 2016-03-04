#include "cpu/exec/template-start.h"

#define instr lgdt

make_helper(concat(lgdt_, SUFFIX)){
    lnaddr_t taddr = instr_fetch(eip + 2, 4);
    uint16_t t16 = lnaddr_read(taddr, 2);
    uint32_t t32 = lnaddr_read(taddr + 2, 4);
    cpu.gdtr = t32;
    cpu.gdtr_limit = t16;
    print_asm("lgdtl 0x%x", taddr);
    return 6;
}

#include "cpu/exec/template-end.h"
