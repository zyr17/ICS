#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args) {
    if (args == NULL){
        cpu_exec(1);
        return 0;
    }
    int times = 1;
    sscanf(args, "%d", &times);
    if (times < 1){
        printf("Wrong input\n");
        return 0;
    }
	cpu_exec(times);
	return 0;
}

static int cmd_info(char *args) {
    if (args == NULL){
        NOCMD_i:;
        printf("info: No command\n");
        return 0;
    }
    char *cmd = strtok(args, " ");
    if(cmd == NULL) goto NOCMD_i;
    if (strcmp(cmd, "r") == 0){
        //eax, ecx, edx, ebx, esp, ebp, esi, edi
        printf("eax\t\t0x%x\n", cpu.eax);
        printf("ecx\t\t0x%x\n", cpu.ecx);
        printf("edx\t\t0x%x\n", cpu.edx);
        printf("ebx\t\t0x%x\n", cpu.ebx);
        printf("esp\t\t0x%x\n", cpu.esp);
        printf("ebp\t\t0x%x\n", cpu.ebp);
        printf("esi\t\t0x%x\n", cpu.esi);
        printf("edi\t\t0x%x\n", cpu.edi);
        return 0;
    }
    printf("Unknown command '%s'\n", cmd);
    return 0;
}

static int cmd_x(char *args) {
    if (args == NULL){
        NOCMD_x:;
        printf("x: No command\n");
        return 0;
    }
    char *cmd = strtok(args, " ");
    if(cmd == NULL) goto NOCMD_x;
    char *cmd2 = strtok(NULL, " ");
    if (cmd2 == NULL){
        printf("x: Expression missed\n");
        return 0;
    }
    int times = - 1;
    sscanf(cmd, "%d", &times);
    if (times < 0){
        printf("x: N input error\n");
        return 0;
    }
    unsigned memaddr = 0;
    if (sscanf(cmd2, "%x", &memaddr) != 1){
        printf("x: expr input error\n");
        return 0;
    }
    int i = 0;
    for (; i < times; i ++ ){
        int now = i + memaddr;
        int number = swaddr_read(now, 1);
        if (i % 4){
            printf("\t%02x", number);
        }
        else{
            if (i && i % 92 == 0){
                printf("\n--Type <return> to continue, or q <return> to quit---");
                for (; ; ){
                    int tch = getchar();
                    if (tch == '\n') break;
                    if (tch == 'q') return 0;
                }
            }
            printf("\n0x%x:\t%02x", now, number);
        }
    }printf("\n");
    return 0;
}

static int cmd_p(char *args) {
    if (args == NULL){
        NOTHING_P:;
        printf("p: Input nothing.\n");
        return 0;
    }
    int suc = 0;
    uint32_t ans = expr(args, &suc);

    #define FAIL 0
    #define SBOO 1
    #define SDIG 2
    #define SHEX 3
    #define SFLO 4

    if (suc == 0){
        printf("p: Calc error.\n");
        return 0;
    }
    else if (suc == - 1)
        goto NOTHING_P;
    union{
        uint32_t a;
        float b;
        int c;
    }tttmp;
    tttmp.a = ans;
    if (suc == SBOO){
        printf("Success:\n");
        if (tttmp.a) printf("bool: true\n");
        else printf("boolean: false\n");
        printf("Hex: 0x%X\n", tttmp.a);
    }
    else if (suc == SDIG){
        printf("Success:\nInt: %d\nHex: 0x%X\n", tttmp.c, tttmp.a);
    }
    else if (suc == SHEX){
        printf("Success:\nHex: 0x%X\nInt: %d\n", tttmp.a, tttmp.c);
    }
    else{
        printf("Success:\nFloat: %.10e\nHex: 0x%X\n", tttmp.b, tttmp.a);
    }
    //printf("Success: %u %f\n", ans, tttmp.b);
    return 0;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{ "si", "[si N] Do N steps and pause. Default N = 1.", cmd_si },
	{ "info", "[info <r,w>] r: to print the register. w(TBC): to print the checkpoint.", cmd_info},
	{ "x", "[x N expr] calculate the expression(TBC), the answer is a address, and print the memory N bits start with that. 4 bits a line.", cmd_x},
	{ "p", "[p expr] calculate the expr and print it.", cmd_p},

	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
                printf("\x1b[32;1m");
				if(cmd_table[i].handler(args) < 0) { printf("\033[0m");return; }
				printf("\033[0m");
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
