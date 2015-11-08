#include "cpu/exec/template-start.h"

#define instr jcc

static void do_execute () {
    int flag, init = swaddr_read(cpu.eip, 1);
    if (init == 0x77) flag = cpu.CF == 0 && cpu.ZF == 0;                  //JA
    else if (init == 0x73) flag = cpu.CF == 0;                            //JAE
    else if (init == 0x72) flag = cpu.CF == 1;                            //JB
    else if (init == 0x76) flag = cpu.CF == 1 && cpu.ZF == 1;             //JBE
    else if (init == 0x72) flag = cpu.CF == 1;                            //JC
    else if (init == 0xE3) flag = cpu.ecx == 0;                           //JCXZ
    else if (init == 0xE3) flag = cpu.ecx == 0;                           //JECXZ
    else if (init == 0x74) flag = cpu.ZF == 1;                            //JE
    else if (init == 0x74) flag = cpu.ZF == 1;                            //JZ
    else if (init == 0x7F) flag = cpu.ZF == 0 && cpu.SF == cpu.OF;        //JG
    else if (init == 0x7D) flag = cpu.SF == cpu.OF;                       //JGE
    else if (init == 0x7C) flag = cpu.SF != cpu.OF;                       //JL
    else if (init == 0x7E) flag = cpu.ZF == 1 || cpu.SF != cpu.OF;        //JLE
    else if (init == 0x76) flag = cpu.CF == 1 && cpu.ZF == 1;             //JNA
    else if (init == 0x72) flag = cpu.CF == 1;                            //JNAE
    else if (init == 0x73) flag = cpu.CF == 0;                            //JNB
    else if (init == 0x77) flag = cpu.CF == 0 && cpu.ZF == 0;             //JNBE
    else if (init == 0x73) flag = cpu.CF == 0;                            //JNC
    else if (init == 0x75) flag = cpu.ZF == 0;                            //JNE
    else if (init == 0x7E) flag = cpu.ZF == 1 || cpu.SF != cpu.OF;        //JNG
    else if (init == 0x7C) flag = cpu.SF != cpu.OF;                       //JNGE
    else if (init == 0x7D) flag = cpu.SF == cpu.OF;                       //JNL
    else if (init == 0x7F) flag = cpu.ZF == 0 && cpu.SF == cpu.OF;        //JNLE
    else if (init == 0x71) flag = cpu.OF == 0;                            //JNO
    else if (init == 0x7B) flag = cpu.PF == 0;                            //JNP
    else if (init == 0x79) flag = cpu.SF == 0;                            //JNS
    else if (init == 0x75) flag = cpu.ZF == 0;                            //JNZ
    else if (init == 0x70) flag = cpu.OF == 1;                            //JO
    else if (init == 0x7A) flag = cpu.PF == 1;                            //JP
    else if (init == 0x7A) flag = cpu.PF == 1;                            //JPE
    else if (init == 0x7B) flag = cpu.PF == 0;                            //JPO
    else if (init == 0x78) flag = cpu.SF == 1;                            //JS
    else if (init == 0x74) flag = cpu.ZF == 1;                            //JZ

    printf("%s ")
	//print_asm_template1();
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"
