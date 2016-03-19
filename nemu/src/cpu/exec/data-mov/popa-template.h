#include "cpu/exec/template-start.h"

#define instr popa


#if DATA_BYTE == 2 || DATA_BYTE == 4
make_helper(concat(popa_, SUFFIX)){
    assert(DATA_BYTE == 4);
    DATA_TYPE tmp = -2;
    tmp = tmp + 2;
    pop_pop(DATA_BYTE, cpu.edi);
    pop_pop(DATA_BYTE, cpu.esi);
    pop_pop(DATA_BYTE, cpu.ebp);
    pop_pop(DATA_BYTE, tmp);
    pop_pop(DATA_BYTE, cpu.ebx);
    pop_pop(DATA_BYTE, cpu.edx);
    pop_pop(DATA_BYTE, cpu.ecx);
    pop_pop(DATA_BYTE, cpu.eax);
    print_asm("popa");
    return 1;
}
#endif

#include "cpu/exec/template-end.h"
