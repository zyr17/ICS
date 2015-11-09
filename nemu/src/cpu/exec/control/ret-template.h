#include "cpu/exec/template-start.h"

#define instr ret

static void do_execute () {
    pop_pop(DATA_BYTE, cpu.eip);
    cpu.esp += op_src -> val;

	print_asm_template1();
}

make_instr_helper(i)

make_helper(concat(ret_0_, SUFFIX)) {
    pop_pop(DATA_BYTE, cpu.eip);

    print_asm("ret" str(SUFFIX));
    return 1;
}

#include "cpu/exec/template-end.h"
