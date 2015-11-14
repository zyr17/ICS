#include "cpu/exec/template-start.h"

#define instr call

static void do_execute () {
    push_push(DATA_BYTE, cpu.eip + DATA_BYTE + 1);

    if (swaddr_read(cpu.eip, 1) == 0xe8) cpu.eip += op_src -> val;
    else{
        cpu.EIP_CHANGEABLE = 0;
        cpu.eip = op_src -> val;
    }

	print_asm_template1();
}

make_instr_helper(i)
make_instr_helper(rm)

#include "cpu/exec/template-end.h"
