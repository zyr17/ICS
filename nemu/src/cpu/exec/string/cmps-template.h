#include "cpu/exec/template-start.h"

#define instr cmps

make_helper(concat(cmps_0_, SUFFIX)) {
    eflags_add_sub(cpu.edi, cpu.esi, 1, DATA_BYTE);
    int change = cpu.DF ? - DATA_BYTE : DATA_BYTE;
    cpu.edi += change;
    cpu.esi += change;
    return 1;
}

#include "cpu/exec/template-end.h"
