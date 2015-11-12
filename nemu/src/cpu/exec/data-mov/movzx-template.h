#include "cpu/exec/template-start.h"

#define instr movzx_b

static void do_execute() {
    DATA_TYPE tv = (uint8_t)op_src -> val;

	OPERAND_W(op_dest, tv);
	print_asm("movzx" str(SUFFIX) " %s,%s", op_src->str, op_dest->str);
}

make_instr_helper(rm2r)

#if DATA_BYTE == 4
make_helper(movzx_w_rm2r_l) {
    int len = decode_rm2r_l(eip + 1);
    DATA_TYPE tv = (uint16_t)op_src -> val;

	OPERAND_W(op_dest, tv);
	print_asm("movzx" str(SUFFIX) " %s,%s", op_src->str, op_dest->str);
	return len + 1;
}
#endif

#include "cpu/exec/template-end.h"
