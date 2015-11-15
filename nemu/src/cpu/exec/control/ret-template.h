#include "cpu/exec/template-start.h"

#define instr ret

#if DATA_BYTE == 2

static void do_execute () {
    pop_pop(DATA_BYTE, cpu.eip);
    cpu.EIP_CHANGEABLE = 0;
    cpu.esp += op_src -> val;

	print_asm_template1();
}

make_instr_helper(i)
#endif

#if DATA_BYTE == 4
make_helper(ret_i_l) {
    int len = decode_i_w(eip + 1);
    pop_pop(DATA_BYTE, cpu.eip);
    cpu.EIP_CHANGEABLE = 0;
    cpu.esp += op_src -> val;

	print_asm_template1();
	return len + 1;
}
#endif

make_helper(concat(ret_0_, SUFFIX)) {
    pop_pop(DATA_BYTE, cpu.eip);
    cpu.EIP_CHANGEABLE = 0;

    print_asm("ret" str(SUFFIX));
    return 1;
}

#include "cpu/exec/template-end.h"
