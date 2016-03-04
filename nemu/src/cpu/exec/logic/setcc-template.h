#include "cpu/exec/template-start.h"
#include <string.h>

#define instr setcc

static void do_execute () {
    int flag, init = instr_fetch(cpu.eip + 1, 1) & 0xf;
    char nowins[10] = {0};
    if (init == 0x7) flag = cpu.CF == 0 && cpu.ZF == 0,           strcpy(nowins, "seta");
    else if (init == 0x3) flag = cpu.CF == 0,                     strcpy(nowins, "setae");
    else if (init == 0x2) flag = cpu.CF == 1,                     strcpy(nowins, "setb");
    else if (init == 0x6) flag = cpu.CF == 1 || cpu.ZF == 1,      strcpy(nowins, "setbe");
    else if (init == 0x2) flag = cpu.CF == 1,                     strcpy(nowins, "setc");
    else if (init == 0x4) flag = cpu.ZF == 1,                     strcpy(nowins, "sete");
    else if (init == 0xF) flag = cpu.ZF == 0 && cpu.SF == cpu.OF, strcpy(nowins, "setg");
    else if (init == 0xD) flag = cpu.SF == cpu.OF,                strcpy(nowins, "setge");
    else if (init == 0xC) flag = cpu.SF != cpu.OF,                strcpy(nowins, "setl");
    else if (init == 0xE) flag = cpu.ZF == 1 || cpu.SF != cpu.OF, strcpy(nowins, "setle");
    else if (init == 0x6) flag = cpu.CF == 1 || cpu.ZF == 1,      strcpy(nowins, "setna");
    else if (init == 0x2) flag = cpu.CF == 1,                     strcpy(nowins, "setnae");
    else if (init == 0x3) flag = cpu.CF == 0,                     strcpy(nowins, "setnb");
    else if (init == 0x7) flag = cpu.CF == 0 && cpu.ZF == 0,      strcpy(nowins, "setnbe");
    else if (init == 0x3) flag = cpu.CF == 0,                     strcpy(nowins, "setnc");
    else if (init == 0x5) flag = cpu.ZF == 0,                     strcpy(nowins, "setne");
    else if (init == 0xE) flag = cpu.ZF == 1 || cpu.SF != cpu.OF, strcpy(nowins, "setng");
    else if (init == 0xC) flag = cpu.SF != cpu.OF,                strcpy(nowins, "setnge");
    else if (init == 0xD) flag = cpu.SF == cpu.OF,                strcpy(nowins, "setnl");
    else if (init == 0xF) flag = cpu.ZF == 0 && cpu.SF == cpu.OF, strcpy(nowins, "setnle");
    else if (init == 0x1) flag = cpu.OF == 0,                     strcpy(nowins, "setno");
    else if (init == 0xB) flag = cpu.PF == 0,                     strcpy(nowins, "setnp");
    else if (init == 0x9) flag = cpu.SF == 0,                     strcpy(nowins, "setns");
    else if (init == 0x5) flag = cpu.ZF == 0,                     strcpy(nowins, "setnz");
    else if (init == 0x0) flag = cpu.OF == 1,                     strcpy(nowins, "seto");
    else if (init == 0xA) flag = cpu.PF == 1,                     strcpy(nowins, "setp");
    else if (init == 0xA) flag = cpu.PF == 1,                     strcpy(nowins, "setpe");
    else if (init == 0xB) flag = cpu.PF == 0,                     strcpy(nowins, "setpo");
    else if (init == 0x8) flag = cpu.SF == 1,                     strcpy(nowins, "sets");
    else if (init == 0x4) flag = cpu.ZF == 1,                     strcpy(nowins, "setz");
    else{
        flag = 0;
        panic("SETcc ERROR!");
    }
    OPERAND_W(op_src, flag);

	//print_asm_template1();
	print_asm("%s" str(SUFFIX) " %s", nowins, op_src->str);
}

make_instr_helper(rm)

#include "cpu/exec/template-end.h"
