#include "cpu/exec/template-start.h"

#define instr lidt

static void do_execute() {
    assert(DATA_BYTE != 2);
    lnaddr_t taddr = op_src -> val;
    uint16_t t16 = lnaddr_read(taddr, 2);
    uint32_t t32 = lnaddr_read(taddr + 2, 4);
    cpu.idtr = t32;
    cpu.idtr_limit = t16;
    print_asm("lidtl 0x%x", taddr);
}

make_instr_helper(rm)

#include "cpu/exec/template-end.h"
