#include "irq.h"
#include "fs.h"

#include <sys/syscall.h>
void serial_printc(char);

void add_irq_handle(int, void (*)(void));
void mm_brk(uint32_t);

static void sys_brk(TrapFrame *tf) {
#ifdef IA32_PAGE
	mm_brk(tf->ebx);
#endif
	tf->eax = 0;
}

void do_syscall(TrapFrame *tf) {
	switch(tf->eax) {
		/* The ``add_irq_handle'' system call is artificial. We use it to
		 * let user program register its interrupt handlers. But this is
		 * very dangerous in a real operating system. Therefore such a
		 * system call never exists in GNU/Linux.
		 */
		case 0:
			cli();
			add_irq_handle(tf->ebx, (void*)tf->ecx);
			sti();
			break;

		case SYS_brk: sys_brk(tf); break;

		case SYS_write:
            if (tf->ebx == 1 || tf->ebx == 2){
                //asm volatile (".byte 0xd6" : : "a"(2), "c"(tf->ecx), "d"(tf->edx));
                int i;
                for (i = 0; i < tf->edx; i ++ )
                    serial_printc(*(char*)(tf->ecx + i));
                        tf->eax = tf->edx;
            }
            else{
                tf->eax = fs_write(tf->ebx, (void*)tf->ecx, tf->edx);
            }
            break;

        case SYS_read:
            if (tf->ebx >= 0 && tf->ebx < 3){
                panic("read stdxxx");
            }
            else{
                tf->eax = fs_read(tf->ebx, (void*)tf->ecx, tf->edx);
            }
            break;

        case SYS_open:
            tf->eax = fs_open((char*)tf->ebx, tf->ecx);
            break;

        case SYS_close:
            tf->eax = fs_close(tf->ebx);
            break;

        case SYS_lseek:
            tf->eax = fs_lseek(tf->ebx, tf->ecx, tf->edx);
            break;

		/* TODO: Add more system calls. */

		default: panic("Unhandled system call: id = %d", tf->eax);
	}
}

