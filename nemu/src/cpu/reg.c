#include "nemu.h"
#include <stdlib.h>
#include <time.h>

CPU_state cpu;

const char *regsl[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};
const char *sreg[] = {"es", "cs", "ss", "ds"};

void reg_test() {
	srand(time(0));
	uint32_t sample[8];
	uint32_t eip_sample = rand();
	cpu.eip = eip_sample;

	int i;
	for(i = R_EAX; i <= R_EDI; i ++) {
		sample[i] = rand();
		reg_l(i) = sample[i];
		assert(reg_w(i) == (sample[i] & 0xffff));
	}

	assert(reg_b(R_AL) == (sample[R_EAX] & 0xff));
	assert(reg_b(R_AH) == ((sample[R_EAX] >> 8) & 0xff));
	assert(reg_b(R_BL) == (sample[R_EBX] & 0xff));
	assert(reg_b(R_BH) == ((sample[R_EBX] >> 8) & 0xff));
	assert(reg_b(R_CL) == (sample[R_ECX] & 0xff));
	assert(reg_b(R_CH) == ((sample[R_ECX] >> 8) & 0xff));
	assert(reg_b(R_DL) == (sample[R_EDX] & 0xff));
	assert(reg_b(R_DH) == ((sample[R_EDX] >> 8) & 0xff));

	assert(sample[R_EAX] == cpu.eax);
	assert(sample[R_ECX] == cpu.ecx);
	assert(sample[R_EDX] == cpu.edx);
	assert(sample[R_EBX] == cpu.ebx);
	assert(sample[R_ESP] == cpu.esp);
	assert(sample[R_EBP] == cpu.ebp);
	assert(sample[R_ESI] == cpu.esi);
	assert(sample[R_EDI] == cpu.edi);

	assert(eip_sample == cpu.eip);
}

uint32_t lnaddr_read(lnaddr_t, size_t);
void sreg_update(int sreg_num){
    assert(cpu.gdtr + sreg_num * 8 + 7 <= cpu.gdtr_limit);
    unsigned long long tmp = (((unsigned long long)lnaddr_read(cpu.gdtr + cpu.sreg[sreg_num] * 8 + 4, 4)) << 32LL) + lnaddr_read(cpu.gdtr + cpu.sreg[sreg_num] * 4, 4);
    Log("%llx", tmp);
    uint32_t tbase = 0;
    uint32_t tlimit = tmp & 0xffff;
    tmp >>= 16;
    tbase = tmp & 0xffffff;
    tmp >>= 24;
    tmp >>= 8;
    tlimit += (tmp & 0xf) << 16;
    tmp >>= 4;
    tmp >>= 3;
    bool G = tmp & 1;
    tmp >>= 1;
    tbase += tmp << 24;
    cpu.sreg_base[sreg_num] = cpu.sreg_limit[sreg_num] = tbase;
    cpu.sreg_limit[sreg_num] += ((tlimit + 1) << (G * 12)) - 1;
}
