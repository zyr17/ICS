#include "cpu/exec/template-start.h"

#define instr movs

make_helper(concat(movs_0_, SUFFIX)) {
    MEM_W(cpu.edi, MEM_R(cpu.esi));
    int change = cpu.DF ? - DATA_BYTE : DATA_BYTE;
    cpu.edi += change;
    cpu.esi += change;
    return 1;
}

#include "cpu/exec/template-end.h"
