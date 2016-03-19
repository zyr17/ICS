#include "cpu/exec/template-start.h"

#define instr pop

static void do_execute () {
    DATA_TYPE result;
    pop_pop(DATA_BYTE == 1 ? 4 : DATA_BYTE, result);
    OPERAND_W(op_src, result);

	print_asm_template1();
}

make_instr_helper(rm)
make_instr_helper(r)

#include "cpu/exec/template-end.h"
