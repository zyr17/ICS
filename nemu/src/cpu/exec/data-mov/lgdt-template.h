#include "cpu/exec/template-start.h"

#define instr lgdt

make_helper(concat(lgdt_, SUFFIX)){
    uint32_t tmp = instr_fetch(eip + 2, 4);
    cpu.gdtr = tmp;
    print_asm("lgdtl 0x%x", tmp);
    return 6;
}

#include "cpu/exec/template-end.h"
