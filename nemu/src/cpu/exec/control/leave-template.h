#include "cpu/exec/template-start.h"

#define instr leave

make_helper(concat(leave_0_, SUFFIX)) {
    cpu.esp = cpu.ebp;
    pop_pop(DATA_BYTE, cpu.ebp);
    print_asm("leave");
    return 1;
}

#include "cpu/exec/template-end.h"
