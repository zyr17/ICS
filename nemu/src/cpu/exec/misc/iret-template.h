#include "cpu/exec/template-start.h"

#define instr iret

#if DATA_BYTE == 2 || DATA_BYTE == 4

make_helper(concat(iret_, SUFFIX)){
    assert(DATA_BYTE == 4);
	cpu.eip = swaddr_read(cpu.esp, 4, 2);
	cpu.esp += 4;
	cpu.CS = swaddr_read(cpu.esp, 4, 2);
	cpu.esp += 4;
    cpu.eflags = swaddr_read(cpu.esp, 4, 2);
	cpu.esp += 4;
    print_asm("iret");
    return 2;
}

#endif

#include "cpu/exec/template-end.h"
