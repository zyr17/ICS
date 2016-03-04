#include "cpu/exec/template-start.h"
#include <string.h>

#define instr cmovcc

static void do_execute () {
    int flag, init = instr_fetch(cpu.eip + 1, 1);
    init &= 0xf;
    char nowins[10] = {0};
    if (init == 0x7) flag = cpu.CF == 0 && cpu.ZF == 0,           strcpy(nowins, "cmova");
    else if (init == 0x3) flag = cpu.CF == 0,                     strcpy(nowins, "cmovae");
    else if (init == 0x2) flag = cpu.CF == 1,                     strcpy(nowins, "cmovb");
    else if (init == 0x6) flag = cpu.CF == 1 || cpu.ZF == 1,      strcpy(nowins, "cmovbe");
    else if (init == 0x2) flag = cpu.CF == 1,                     strcpy(nowins, "cmovc");
    else if (init == 0x4) flag = cpu.ZF == 1,                     strcpy(nowins, "cmove");
    else if (init == 0xF) flag = cpu.ZF == 0 && cpu.SF == cpu.OF, strcpy(nowins, "cmovg");
    else if (init == 0xD) flag = cpu.SF == cpu.OF,                strcpy(nowins, "cmovge");
    else if (init == 0xC) flag = cpu.SF != cpu.OF,                strcpy(nowins, "cmovl");
    else if (init == 0xE) flag = cpu.ZF == 1 || cpu.SF != cpu.OF, strcpy(nowins, "cmovle");
    else if (init == 0x6) flag = cpu.CF == 1 || cpu.ZF == 1,      strcpy(nowins, "cmovna");
    else if (init == 0x2) flag = cpu.CF == 1,                     strcpy(nowins, "cmovnae");
    else if (init == 0x3) flag = cpu.CF == 0,                     strcpy(nowins, "cmovnb");
    else if (init == 0x7) flag = cpu.CF == 0 && cpu.ZF == 0,      strcpy(nowins, "cmovnbe");
    else if (init == 0x3) flag = cpu.CF == 0,                     strcpy(nowins, "cmovnc");
    else if (init == 0x5) flag = cpu.ZF == 0,                     strcpy(nowins, "cmovne");
    else if (init == 0xE) flag = cpu.ZF == 1 || cpu.SF != cpu.OF, strcpy(nowins, "cmovng");
    else if (init == 0xC) flag = cpu.SF != cpu.OF,                strcpy(nowins, "cmovnge");
    else if (init == 0xD) flag = cpu.SF == cpu.OF,                strcpy(nowins, "cmovnl");
    else if (init == 0xF) flag = cpu.ZF == 0 && cpu.SF == cpu.OF, strcpy(nowins, "cmovnle");
    else if (init == 0x1) flag = cpu.OF == 0,                     strcpy(nowins, "cmovno");
    else if (init == 0xB) flag = cpu.PF == 0,                     strcpy(nowins, "cmovnp");
    else if (init == 0x9) flag = cpu.SF == 0,                     strcpy(nowins, "cmovns");
    else if (init == 0x5) flag = cpu.ZF == 0,                     strcpy(nowins, "cmovnz");
    else if (init == 0x0) flag = cpu.OF == 1,                     strcpy(nowins, "cmovo");
    else if (init == 0xA) flag = cpu.PF == 1,                     strcpy(nowins, "cmovp");
    else if (init == 0xA) flag = cpu.PF == 1,                     strcpy(nowins, "cmovpe");
    else if (init == 0xB) flag = cpu.PF == 0,                     strcpy(nowins, "cmovpo");
    else if (init == 0x8) flag = cpu.SF == 1,                     strcpy(nowins, "cmovs");
    else if (init == 0x4) flag = cpu.ZF == 1,                     strcpy(nowins, "cmovz");
    else{
        flag = 0;
        panic("cmovcc ERROR!");
    }

    if (flag == 1){
        OPERAND_W(op_dest, op_src->val);
    }

	print_asm("%s" str(SUFFIX) " %s", nowins, op_src->str);
}

make_instr_helper(rm2r)

#include "cpu/exec/template-end.h"
