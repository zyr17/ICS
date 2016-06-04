#include "cpu/exec/template-start.h"
#include "device/port-io.h"

#define instr in


static void do_execute() {
	REG(0) = pio_read(op_src->val, DATA_BYTE);
	print_asm_template2();
}

make_instr_helper(i2a)

make_helper(concat(in_, SUFFIX)) {
    REG(0) = pio_read(cpu.gpr[2]._16, DATA_BYTE);

	print_asm("in" str(SUFFIX) " %%%s,[%%dx]", REG_NAME(R_EAX));
	return 1;
}

#include "cpu/exec/template-end.h"
