#include "cpu/exec/template-start.h"

#define instr lgdt

static void do_execute() {
	cpu.gdtr = op_src -> val;
	print_asm_template1();
}

make_instr_helper(rm)

#include "cpu/exec/template-end.h"