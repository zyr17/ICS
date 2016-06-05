#include "common.h"
#include "memory.h"
#include <string.h>

#define VMEM_ADDR 0xa0000
#define SCR_SIZE (320 * 200)

/* Use the function to get the start address of user page directory. */
inline PDE* get_updir();

PTE Vmem[NR_PTE] align_to_page;

void create_video_mapping() {
	/* TODO: create an identical mapping from virtual memory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) to physical memory area
	 * [0xa0000, 0xa0000 + SCR_SIZE) for user program. You may define
	 * some page tables to create this mapping.
	 */
    PDE* u_pde = get_updir();
    //Log("%d", u_pde->val);
    assert(!(u_pde->present));
    u_pde->present = 1;
    u_pde->page_frame = ((int)Vmem) >> 12;
    Log("0x%x, %d", u_pde->page_frame, SCR_SIZE);set_bp();
    int i;
    for (i = 0; i < (SCR_SIZE - 1) / 4096 + 1; i ++ ){
        Vmem[i].present = 1;
        Vmem[i].page_frame = (0xa0000 + i * 4096) >> 12;
    }set_bp();
	//panic("please implement me");
}

void video_mapping_write_test() {
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		buf[i] = i;
	}
}

void video_mapping_read_test() {
	int i;
	uint32_t *buf = (void *)VMEM_ADDR;
	for(i = 0; i < SCR_SIZE / 4; i ++) {
		assert(buf[i] == i);
	}
}

void video_mapping_clear() {
	memset((void *)VMEM_ADDR, 0, SCR_SIZE);
}

