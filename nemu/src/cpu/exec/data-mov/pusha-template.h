#include "cpu/exec/template-start.h"

#define instr pusha


#if DATA_BYTE == 2 || DATA_BYTE == 4
make_helper(concat(pusha_, SUFFIX)){
    assert(DATA_BYTE == 4);
    DATA_TYPE tmp = cpu.esp;
    push_push(DATA_BYTE, cpu.eax);
    push_push(DATA_BYTE, cpu.ecx);
    push_push(DATA_BYTE, cpu.edx);
    push_push(DATA_BYTE, cpu.ebx);
    push_push(DATA_BYTE, tmp);
    push_push(DATA_BYTE, cpu.ebp);
    push_push(DATA_BYTE, cpu.esi);
    push_push(DATA_BYTE, cpu.edi);
    print_asm("pusha");
    return 1;
}
#endif

#include "cpu/exec/template-end.h"
