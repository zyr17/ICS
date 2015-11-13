#define instr cxtx

make_helper(cxtx_0_w){
    if (reg_w(0) < 0) reg_w(2) = 0xffff;
    else reg_w(2) = 0;
    return 1;
}

make_helper(cxtx_0_l){
    if (cpu.eax < 0) cpu.edx = 0xffffffff;
    else cpu.edx = 0;
    return 1;
}
