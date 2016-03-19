#include "cpu/exec/template-start.h"

#define instr lidt

make_helper(concat(lidt_, SUFFIX)){
    assert(DATA_BYTE != 2);
    lnaddr_t taddr = instr_fetch(eip + 2, 4);
    uint16_t t16 = lnaddr_read(taddr, 2);
    uint32_t t32 = lnaddr_read(taddr + 2, 4);
    cpu.idtr = t32;
    cpu.idtr_limit = t16;
    print_asm("lidtl 0x%x", taddr);
    return 6;
}

#include "cpu/exec/template-end.h"
