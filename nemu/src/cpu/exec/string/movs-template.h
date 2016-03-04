#include "cpu/exec/template-start.h"

#define instr movs

make_helper(concat(movs_0_, SUFFIX)) {
    MEM_W(cpu.edi, MEM_R(cpu.esi, 3), 0);
    int change = cpu.DF ? - DATA_BYTE : DATA_BYTE;
    cpu.edi += change;
    cpu.esi += change;
    print_asm("movs %%es:(%%edi),%%ds:(%%esi)");
    return 1;
}

#include "cpu/exec/template-end.h"
