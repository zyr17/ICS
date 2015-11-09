#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

#define C_COLOR printf("\x1b[32;1m");

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
        printf("eip\t\t0x%x\n", cpu.eip);
        printf("eflags\t\t0x%x\nDetails for eflags:\n ... VM RF  . NT IOPL OF DF IF TF SF ZF  . AF  . PF  . CF\n", cpu.eflags);
        printf(" ..0  %d  %d  0  %d   %d%d  %d  %d  %d  %d  %d  %d  0  %d  0  %d  1  %d\n",
               cpu.VM, cpu.RF, cpu.NT, cpu.IOPL / 2, cpu.IOPL & 1, cpu.OF, cpu.DF, cpu.IF, cpu.TF, cpu.SF, cpu.ZF, cpu.AF, cpu.PF, cpu.CF);
        return 0;
    }
    else if (strcmp(cmd, "w") == 0){
        char *tmp = strtok(NULL, " ");
        if (tmp != NULL){
            int num = - 1;
            sscanf(tmp, "%d", &num);
            if (num < 0){
                printf("info w: N input error.\n");
                return 0;
            }
            WP *now = find_wp(num);
            if (now == NULL){
                printf("info w: cannot find watchpoint with NO %d.\n", num);
                return 0;
            }
            printf("Watchpoint details\nNO\t%d\nExpr\t%s\nvalue\t", (*now).NO, (*now).e);
            if ((*now).success == SDIG) printf("%d\n", (*now).number.a);
            else if ((*now).success == SHEX) printf("0x%X\n", (*now).number.b);
            else if ((*now).success == SFLO) printf("%f\n", (*now).number.c);
            else if ((*now).number.a) printf("true\n");
            else printf("false\n");
            return 0;

        }
        output_wp();
        return 0;
    }
    printf("Unknown command '%s'\n", cmd);
    return 0;
}

static int cmd_x_str(char *args, const char *istr, int type) {
    if (args == NULL){
        NOCMD_x:;
        printf("%s: No command\n", istr);
        return 0;
    }
    char *cmd = strtok(args, " ");
    if(cmd == NULL) goto NOCMD_x;
    char *cmd2 = cmd;
    for (; *cmd2; cmd2 ++ );
    for (cmd2 ++ ; *cmd2 == ' '; cmd2 ++ );
    if (*cmd2 == 0){
        printf("x: Expression missed\n");
        return 0;
    }
    int times = - 1;
    sscanf(cmd, "%d", &times);
    if (times < 0){
        printf("x: N input error\n");
        return 0;
    }
    #define MAXADD 134217727
    uint32_t memaddr = 0;
    int suc = 0;
    memaddr = expr(cmd2, &suc);
    C_COLOR
    if (suc == - 1){
        printf("x: expr input error\n");
        return 0;
    }
    else if (suc == 0){
        printf("x: Calc error.\n");
        return 0;
    }
    else if (suc == SFLO){
        printf("x: expr get a float.\n");
        return 0;
    }
    uint32_t i = 0;
    for (; i < times; i ++ ){
        uint32_t now = i + memaddr;
        if (now >> 27){
            printf("\x1b[31;1m\nOut of memory!\n");
            return 0;
        }
        int number = swaddr_read(now, 1);
        if (i % 4){
            if (type) printf("\t%02x", number);
            else printf("\t\'%c\'", number);
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
            if (i) printf("\n");
            if (type) printf("0x%x:\t%02x", now, number);
            else printf("0x%x:\t\'%c\'", now, number);
        }
    }printf("\n");
    return 0;
}

static int cmd_x(char *args) {
    return cmd_x_str(args, "x", 1);
}
static int cmd_str(char *args) {
    return cmd_x_str(args, "str", 0);
}

static int cmd_p(char *args) {
    if (args == NULL){
        NOTHING_P:;
        printf("p: Input nothing.\n");
        return 0;
    }
    int suc = 0;
    uint32_t ans = expr(args, &suc);
    C_COLOR
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

static int cmd_w(char *args) {
    int len, i, suc;
    if (args == NULL){
        NOTHING_W:;
        printf("w: Input nothing.\n");
        return 0;
    }
    len = strlen(args);
    for (i = 0; i < len; i ++ )
        if (args[i] > 32) goto BIBIBABA_W;
    goto NOTHING_W;
    BIBIBABA_W:;
    if (len >= __WATCHPOINT_CLEN__){
        printf("w: Expr too long.\n");
        return 0;
    }
    uint32_t tnum = expr(args, &suc);
    C_COLOR;
    if (suc == FAIL){
        printf("w: Expr illegal.\n");
        return 0;
    }
    WP *now = new_wp();
    if (now == NULL) return 0;
    memcpy((*now).e, args, len + 1);
    (*now).success = suc;
    (*now).number.b = tnum;
    printf("w: Success, watchpoint NO = %d, now Expr = ", (*now).NO);
    if (suc == SDIG) printf("%d\n", (*now).number.a);
    else if (suc == SHEX) printf("0x%X\n", (*now).number.b);
    else if (suc == SFLO) printf("%f\n", (*now).number.c);
    else if (suc == SBOO){
        if ((*now).number.b) printf("true\n");
        else printf("false\n");
    }
    else printf("???\n");
    return 0;
}

static int cmd_d(char *args) {
    int tmp = - 1, i, len;
    if (args == NULL){
        NOTHING_D:;
        printf("d: Input nothing.\n");
        return 0;
    }
    len = strlen(args);
    for (i = 0; i < len; i ++ )
        if (args[i] > 32) goto BIBIBABA_D;
    goto NOTHING_D;
    BIBIBABA_D:;
    sscanf(args, "%d", &tmp);
    if (tmp < 0){
        printf("d: N input error.\n");
        return 0;
    }
    WP *pos = find_wp(tmp);
    if (pos == NULL){
        printf("d: NO %d not found.\n", tmp);
        return 0;
    }
    printf("d: Success, deleted watchpoint with NO = %d, expr = %s.\n", tmp, (*pos).e);
    free_wp(pos);
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
	{ "info", "[info r][info w (N)] r: to print the register. w: to print the checkpoint(s). w N: to show details about Nth checkpoint.", cmd_info},
	{ "x", "[x N expr] calculate the expression, the answer is a address, and print the memory N bits start with that. 4 bits a line.", cmd_x},
	{ "str", "[str N expr] the same as command \"x\", but output the number as char.", cmd_str},
	{ "p", "[p expr] calculate the expr and print it in several ways.", cmd_p},
	{ "w", "[w expr] set a watchpoint, when expr changes, stop.", cmd_w},
	{ "d", "[d N] delete the Nth watchpoint.", cmd_d},

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
                C_COLOR;
				if(cmd_table[i].handler(args) < 0) { printf("\033[0m");return; }
				printf("\033[0m");
				break;
			}
		}

		if(i == NR_CMD) { printf("\x1b[32;1mUnknown command '%s'\n\033[0m", cmd); }
	}
}
