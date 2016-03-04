#include "cpu/exec/template-start.h"

#define instr lgdt

make_helper(concat(lgdt_, SUFFIX)){
    uint16_t t16 = instr_fetch(eip + 2, 2);
    uint32_t t32 = instr_fetch(eip + 4, 4);
    cpu.gdtr = t32;
    cpu.gdtr_limit = t16;
    print_asm("lgdtl 0x%x", t32);
    return 6;
}

#include "cpu/exec/template-end.h"
