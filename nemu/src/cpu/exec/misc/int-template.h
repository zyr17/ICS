#include "cpu/exec/template-start.h"

#define instr int

#if DATA_BYTE == 2 || DATA_BYTE == 4

void raise_intr(uint8_t);
make_helper(concat(int_i_, SUFFIX)){
    assert(DATA_BYTE == 4);
    uint8_t tmp = instr_fetch(eip + 1, 1);
    raise_intr(tmp);
    print_asm_template1();
    return 2;
}

#endif

#include "cpu/exec/template-end.h"
