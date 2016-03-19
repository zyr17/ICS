#include <setjmp.h>
#include "cpu/reg.h"
#include "memory/memory.h"
extern jmp_buf jbuf;

void sreg_update(uint32_t);
void raise_intr(uint8_t NO) {
	/* TODO: Trigger an interrupt/exception with ``NO''.
	 * That is, use ``NO'' to index the IDT.
	 */

	swaddr_write(cpu.esp -= 4, 4, cpu.eflags, 2);
	swaddr_write(cpu.esp -= 4, 4, cpu.CS, 2);
	swaddr_write(cpu.esp -= 4, 4, cpu.eip + 2, 2);

    lnaddr_t nowk = cpu.idtr + (NO & 0xf8);
    unsigned long long tmp = ((long long)lnaddr_read(nowk + 4, 4) << 32LL) + lnaddr_read(nowk, 4);
    cpu.CS = (tmp >> 16) & 0xffff;
    sreg_update(1);
    swaddr_t addr = ((tmp >> 32) & 0xffff0000) + (tmp & 0xffff) + cpu.sreg_base[1];
    assert(addr <= cpu.sreg_limit[1]);
    cpu.eip = addr;

	/* Jump back to cpu_exec() */
	longjmp(jbuf, 1);
}
