#define instr cxtx

make_helper(cxtx_0_w){
    if ((int16_t)reg_w(0) < 0) reg_w(2) = 0xffff;
    else reg_w(2) = 0;
    print_asm("cwtl");
    return 1;
}

make_helper(cxtx_0_l){
    if ((int32_t)cpu.eax < 0) cpu.edx = 0xffffffff;
    else cpu.edx = 0;
    print_asm("cltd");
    return 1;
}
