#include "cpu/exec/template-start.h"
#include "device/port-io.h"

#define instr out


static void do_execute() {
	pio_write(op_src->val, DATA_BYTE, REG(0));
	print_asm("out" str(SUFFIX) " 0x%x,%%%s", op_src->val, REG_NAME(R_EAX));
}

make_instr_helper(i2a)

make_helper(concat(out_, SUFFIX)) {
    pio_write(cpu.gpr[2]._16, DATA_BYTE, REG(0));

	print_asm("out" str(SUFFIX) " [%%dx],%%%s", REG_NAME(R_EAX));
	return 1;
}

#include "cpu/exec/template-end.h"
