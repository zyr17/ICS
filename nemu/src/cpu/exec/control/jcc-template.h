#include "cpu/exec/template-start.h"
#include <string.h>

#define instr jcc

static void do_execute () {
    int flag, init = swaddr_read(cpu.eip, 1) & 0xf;
    if (init == 0x0f) init = swaddr_read(cpu.eip + 1, 1) & 0xf;
    char nowins[10] = {0};
    if (swaddr_read(cpu.eip, 1) == 0xE3) flag = cpu.ecx == 0,     strcpy(nowins, "jcxz");
    else if (swaddr_read(cpu.eip, 1) == 0xE3) flag = cpu.ecx == 0,strcpy(nowins, "jecxz");
    else if (init == 0x7) flag = cpu.CF == 0 && cpu.ZF == 0,      strcpy(nowins, "ja");
    else if (init == 0x3) flag = cpu.CF == 0,                     strcpy(nowins, "jae");
    else if (init == 0x2) flag = cpu.CF == 1,                     strcpy(nowins, "jb");
    else if (init == 0x6) flag = cpu.CF == 1 || cpu.ZF == 1,      strcpy(nowins, "jbe");
    else if (init == 0x2) flag = cpu.CF == 1,                     strcpy(nowins, "jc");
    else if (init == 0x4) flag = cpu.ZF == 1,                     strcpy(nowins, "je");
    else if (init == 0x4) flag = cpu.ZF == 1,                     strcpy(nowins, "jz");
    else if (init == 0xF) flag = cpu.ZF == 0 && cpu.SF == cpu.OF, strcpy(nowins, "jg");
    else if (init == 0xD) flag = cpu.SF == cpu.OF,                strcpy(nowins, "jge");
    else if (init == 0xC) flag = cpu.SF != cpu.OF,                strcpy(nowins, "jl");
    else if (init == 0xE) flag = cpu.ZF == 1 || cpu.SF != cpu.OF, strcpy(nowins, "jle");
    else if (init == 0x6) flag = cpu.CF == 1 || cpu.ZF == 1,      strcpy(nowins, "jna");
    else if (init == 0x2) flag = cpu.CF == 1,                     strcpy(nowins, "jnae");
    else if (init == 0x3) flag = cpu.CF == 0,                     strcpy(nowins, "jnb");
    else if (init == 0x7) flag = cpu.CF == 0 && cpu.ZF == 0,      strcpy(nowins, "jnbe");
    else if (init == 0x3) flag = cpu.CF == 0,                     strcpy(nowins, "jnc");
    else if (init == 0x5) flag = cpu.ZF == 0,                     strcpy(nowins, "jne");
    else if (init == 0xE) flag = cpu.ZF == 1 || cpu.SF != cpu.OF, strcpy(nowins, "jng");
    else if (init == 0xC) flag = cpu.SF != cpu.OF,                strcpy(nowins, "jnge");
    else if (init == 0xD) flag = cpu.SF == cpu.OF,                strcpy(nowins, "jnl");
    else if (init == 0xF) flag = cpu.ZF == 0 && cpu.SF == cpu.OF, strcpy(nowins, "jnle");
    else if (init == 0x1) flag = cpu.OF == 0,                     strcpy(nowins, "jno");
    else if (init == 0xB) flag = cpu.PF == 0,                     strcpy(nowins, "jnp");
    else if (init == 0x9) flag = cpu.SF == 0,                     strcpy(nowins, "jns");
    else if (init == 0x5) flag = cpu.ZF == 0,                     strcpy(nowins, "jnz");
    else if (init == 0x0) flag = cpu.OF == 1,                     strcpy(nowins, "jo");
    else if (init == 0xA) flag = cpu.PF == 1,                     strcpy(nowins, "jp");
    else if (init == 0xA) flag = cpu.PF == 1,                     strcpy(nowins, "jpe");
    else if (init == 0xB) flag = cpu.PF == 0,                     strcpy(nowins, "jpo");
    else if (init == 0x8) flag = cpu.SF == 1,                     strcpy(nowins, "js");
    else if (init == 0x4) flag = cpu.ZF == 1,                     strcpy(nowins, "jz");
    else{
        flag = 0;
        panic("Jcc ERROR!");
    }

    if (flag == 1){
        DATA_TYPE_S tmp =  op_src -> val;
        cpu.eip += (int32_t) tmp;
        if (DATA_BYTE == 2) cpu.eip &= 0xffff;
    }

    /*if (DATA_BYTE == 1) printf("%8x:   %2x %2x                                 %s $0x%x\n",
                        cpu.eip, swaddr_read(cpu.eip, 1), swaddr_read(cpu.eip + 1, 1), nowins, swaddr_read(cpu.eip + 1, 1));
    else if (DATA_BYTE == 2) printf("%8x:   %2x %2x %2x                              %s $0x%x\n",
                             cpu.eip, swaddr_read(cpu.eip, 1), swaddr_read(cpu.eip + 1, 1), swaddr_read(cpu.eip + 2, 1), nowins, swaddr_read(cpu.eip + 1, 2));
    else if (DATA_BYTE == 4) printf("%8x:   %2x %2x %2x %2x %2x                        %s $0x%x\n",
                             cpu.eip, swaddr_read(cpu.eip, 1), swaddr_read(cpu.eip + 1, 1), swaddr_read(cpu.eip + 2, 1),
                             swaddr_read(cpu.eip + 3, 1), swaddr_read(cpu.eip + 4, 1), nowins, swaddr_read(cpu.eip + 1, 4));*/
	//print_asm_template1();
	print_asm("%s" str(SUFFIX) " %s", nowins, op_src->str);
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"
