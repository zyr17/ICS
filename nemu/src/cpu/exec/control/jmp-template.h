#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute () {

    DATA_TYPE_S tmp =  op_src -> val;
    if (swaddr_read(cpu.eip, 1) == 0xff){
        cpu.eip = tmp;
        cpu.EIP_CHANGEABLE = 0;
    }
    else cpu.eip += (int32_t) tmp;
    //if (DATA_BYTE == 2) cpu.eip &= 0xffff;
    if (DATA_BYTE == 2) panic("jmp rel16/rm16!!");

	print_asm_template1();
}

make_instr_helper(i)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(rm)
#endif

#include "cpu/exec/template-end.h"
