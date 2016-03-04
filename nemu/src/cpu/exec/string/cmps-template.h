#include "cpu/exec/template-start.h"

#define instr cmps

make_helper(concat(cmps_0_, SUFFIX)) {
    eflags_add_sub(swaddr_read(cpu.esi, DATA_BYTE, 3), swaddr_read(cpu.edi, DATA_BYTE, 0), 1, DATA_BYTE);
    int change = cpu.DF ? - DATA_BYTE : DATA_BYTE;
    cpu.edi += change;
    cpu.esi += change;
    print_asm("cmps %%es:(%%edi),%%ds:(%%esi)");
    return 1;
}

#include "cpu/exec/template-end.h"
