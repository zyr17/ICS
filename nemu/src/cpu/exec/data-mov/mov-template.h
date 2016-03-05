#include "cpu/exec/template-start.h"

#define instr mov

static void do_execute() {
	OPERAND_W(op_dest, op_src->val);
	print_asm_template2();
}

make_instr_helper(i2r)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)

make_helper(concat(mov_a2moffs_, SUFFIX)) {
	swaddr_t addr = instr_fetch(eip + 1, 4);
	MEM_W(addr, REG(R_EAX), 3);

	print_asm("mov" str(SUFFIX) " %%%s,0x%x", REG_NAME(R_EAX), addr);
	return 5;
}

make_helper(concat(mov_moffs2a_, SUFFIX)) {
	swaddr_t addr = instr_fetch(eip + 1, 4);
	REG(R_EAX) = MEM_R(addr, 3);

	print_asm("mov" str(SUFFIX) " 0x%x,%%%s", addr, REG_NAME(R_EAX));
	return 5;
}

#if DATA_BYTE == 2

void sreg_update(int);
make_helper(mov_sreg){
    uint8_t modrm = instr_fetch(eip + 1, 1);
    int reg_num = modrm & 0x7;
    int sreg_num = (modrm >> 3) & 0x7;
    if (instr_fetch(eip, 1) == 0x8c){
        reg_w(reg_num) = cpu.sreg[sreg_num];
        print_asm("mov" str(SUFFIX) " %%%s,%%%s", SREG_NAME(sreg_num), REG_NAME(R_EAX));
    }
    else{
        cpu.sreg[sreg_num] = reg_w(reg_num);
        sreg_update(sreg_num);
        print_asm("mov" str(SUFFIX) " %%%s,%%%s", REG_NAME(R_EAX), SREG_NAME(sreg_num));
    }
    return 2;
}

#endif

#if DATA_BYTE == 4

make_helper(mov_crx){Log("mov_crx");
    uint8_t modrm = instr_fetch(eip + 1, 1);
    int reg_num = modrm & 0x7;
    int cr_num = (modrm >> 3) & 0x7;
    if (instr_fetch(eip, 1) == 0x20){
        reg_l(reg_num) = cpu.cr[cr_num];
        print_asm("mov" str(SUFFIX) " %%cr%d,%%%s", cr_num, REG_NAME(R_EAX));
    }
    else{
        cpu.cr[cr_num] = reg_l(reg_num);
        print_asm("mov" str(SUFFIX) " %%%s,%%cr%d", REG_NAME(R_EAX), cr_num);
    }
    return 2;
}

#endif

#include "cpu/exec/template-end.h"
