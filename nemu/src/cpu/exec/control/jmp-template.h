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

#if DATA_BYTE == 1
void sreg_update(int);
make_helper(ljmp){
    uint32_t t32 = instr_fetch(eip + 1, 4);
    uint16_t t16 = instr_fetch(eip + 5, 2);
    cpu.CS = t16;
    sreg_update(1);
    cpu.eip = t32;
    cpu.EIP_CHANGEABLE = 0;
    print_asm("mov" str(SUFFIX) " $0x%x,$0x%x", t16, t32);
    return 7;
}
#endif

#include "cpu/exec/template-end.h"
