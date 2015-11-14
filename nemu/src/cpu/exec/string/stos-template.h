#include "cpu/exec/template-start.h"

#define instr stos

make_helper(concat(stos_0_, SUFFIX)) {
    MEM_W(cpu.edi, cpu.eax);
    if (cpu.DF) cpu.edi -= DATA_BYTE;
    else cpu.edi += DATA_BYTE;
    return 1;
}

#include "cpu/exec/template-end.h"
