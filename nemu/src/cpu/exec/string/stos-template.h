#include "cpu/exec/template-start.h"

#define instr stos

make_helper(concat(stos_0_, SUFFIX)) {
    MEM_W(cpu.edi, cpu.eax, 0);
    if (cpu.DF) cpu.edi -= DATA_BYTE;
    else cpu.edi += DATA_BYTE;
    print_asm("stos %%al,%%es:(%%edi)");
    return 1;
}

#include "cpu/exec/template-end.h"
