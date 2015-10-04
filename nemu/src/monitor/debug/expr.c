#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum {
	NOTYPE = 256, EQ, LL, RR, BEQ, SEQ, NEQ,
    BAA, BOO, REG, HEX, DIG, VAR, FLOAT, LTR,

	/* TODO: Add more token types */

};

int ff[300];

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ},						// equal
	{"<<", LL},
	{">>", RR},
	{">=", BEQ},
	{"<=", SEQ},
	{"!=", NEQ},
	{"&&", BAA},
	{"\\|\\|", BOO},
	{"\\(", '('},
	{"\\)", ')'},
	{"-", '-'},
	{"\\*", '*'},
    {"/", '/'},
    {"!", '!'},
    {"~", '~'},
    {"%", '%'},
    {"<", '<'},
    {">", '>'},
    {"&", '&'},
    {"\\^", '^'},
    {"\\|", '|'},
    {"\\?", '?'},
    {":", ':'},
    {"\'[^\']+\'", LTR},
    {"\\$[qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPLKJHGFDSAZXCVBNM]+", REG},
    {"([1234567890]+\\.|[1234567890]+\\.[1234567890]+|\\.[1234567890]+)([eE][\\+-]{0,1}[1234567890]*){0,1}|[0123456789]+[eE][\\+-]{0,1}[0123456789]*", FLOAT},
    {"0[xX][0123456789dabcdefABCDEF]+", HEX},
    {"[1234567980][qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPLKJHGFDSAZXCVBNM1234567890]*", DIG},
    {"[qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPLKJHGFDSAZXCVBNM][qwertyuioplkjhgfdsazxcvbnmQWERTYUIOPLKJHGFDSAZXCVBNM1234567890]*", VAR},

};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;
	ff['?'] = ff[':'] = 1;
    ff[BOO] = 2;
    ff[BAA] = 3;
    ff['|'] = 4;
    ff['^'] = 5;
    ff['&'] = 6;
    ff[NEQ] = ff[EQ] = 7;
    ff[BEQ] = ff[SEQ] = ff['<'] = ff['>'] = 8;
    ff[LL] = ff[RR] = 9;
    ff['+'] = ff['-'] = 10;
    ff['*'] = ff['/'] = ff['%'] = 11;
    ff['!'] = ff['~'] = ff['&'] = 12;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

#define TOKEN_LEN 32
#define TOKEN_TOT 32
#define BRACKET_STEP 15
#define MAXX 2100000000

typedef struct token {
	int type;
	char str[TOKEN_LEN];
} Token;

Token tokens[TOKEN_TOT];
int nr_token;
int prio[TOKEN_TOT];

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	int bup = 0;

	nr_token = 0;
    prio[0] = MAXX - 1;

	while(e[position] != '\0') {//Log("%d\n", position);
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {//Log("%d %s\n", position, rules[i].regex);
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s, |%d|", i, rules[i].regex, position, substr_len, substr_len, substr_start, nr_token);
				position += substr_len;
				if (substr_len >= TOKEN_LEN){
                    e[position] = 0;
                    printf("Token too long: %s\n", substr_start);
                    return false;
				}
				if (rules[i].token_type == NOTYPE)
                    break;
				if (nr_token >= TOKEN_TOT - 1){
                    printf("Too many tokens.\n");
                    return false;
				}
				nr_token ++ ;
				if (rules[i].token_type == '(') bup += BRACKET_STEP;
                memcpy(tokens[nr_token].str, substr_start, substr_len);
                tokens[nr_token].str[substr_len] = 0;
                tokens[nr_token].type = rules[i].token_type;
                if (rules[i].token_type == DIG ||
                    rules[i].token_type == HEX ||
                    rules[i].token_type == FLOAT ||
                    rules[i].token_type == LTR ||
                    rules[i].token_type == REG ||
                    rules[i].token_type == VAR) prio[nr_token] = MAXX;
                else{
                    prio[nr_token] = bup + ff[rules[i].token_type];
                    if (rules[i].token_type != '(' && rules[i].token_type != ')')
                        if (nr_token == 1 || ff[tokens[nr_token - 1].type] > 0)
                            prio[nr_token] = bup + ff['~'];
                }
                printf("Type priority: %d\n", prio[nr_token]);
				if (rules[i].token_type == ')') bup -= BRACKET_STEP;


				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array ``tokens''. For certain
				 * types of tokens, some extra actions should be performed.
				 */

				/*switch(rules[i].token_type) {
					default: panic("please implement me");
				}*/

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

#define FAIL 0
#define SBOO 1
#define SDIG 2
#define SHEX 3
#define SFLO 4

#define max(A,B) (((A)>(B))?(A):(B))

void Type_convert(const int totype, int *nowsuc, Token *ttok){
    if (*nowsuc == totype) return;
    if (*nowsuc == FAIL) return;
    int now = *nowsuc;
    *nowsuc = totype;
    if (totype == SDIG && now == SHEX){
        union{
            uint32_t x;
            int y;
        }ttt;
        sscanf((*ttok).str, "%x", &ttt.x);
        sprintf((*ttok).str, "%d", ttt.y);
        return;
    }
    if (totype == SDIG && now == SFLO){
        float ttt;
        sscanf((*ttok).str, "%f", &ttt);
        sprintf((*ttok).str, "%d", (int)ttt);
        return;
    }
    if (totype == SDIG && now == SBOO){
        int ttt = (*ttok).str[0] ? 1 : 0;
        sprintf((*ttok).str, "%d", ttt);
        return;
    }
    if (totype == SHEX && now == SDIG){
        union{
            uint32_t x;
            int y;
        }ttt;
        sscanf((*ttok).str, "%d", &ttt.y);
        sprintf((*ttok).str, "0x%x", ttt.x);
        return;
    }
    if (totype == SHEX && now == SFLO){
        float ttt;
        sscanf((*ttok).str, "%f", &ttt);
        sprintf((*ttok).str, "0x%x", (uint32_t)ttt);
        return;
    }
    if (totype == SHEX && now == SBOO){
        int ttt = (*ttok).str[0] ? 1 : 0;
        sprintf((*ttok).str, "0x%x", ttt);
        return;
    }
    if (totype == SFLO && now == SDIG){
        int ttt;
        sscanf((*ttok).str, "%d", &ttt);
        sprintf((*ttok).str, "%.20e", (float)ttt);
        return;
    }
    if (totype == SFLO && now == SHEX){
        uint32_t ttt;
        sscanf((*ttok).str, "%x", &ttt);
        sprintf((*ttok).str, "%.20e", (float)ttt);
        return;
    }
    if (totype == SFLO && now == SBOO){
        int ttt = (*ttok).str[0] ? 1 : 0;
        sprintf((*ttok).str, "%.20e", (float)ttt);
        return;
    }
    if (totype == SBOO && now == SDIG){
        int ttt;
        sscanf((*ttok).str, "%d", &ttt);
        if (ttt) sprintf((*ttok).str, "true");
        else (*ttok).str[0] = 0;
        return;
    }
    if (totype == SBOO && now == SHEX){
        uint32_t ttt;
        sscanf((*ttok).str, "%x", &ttt);
        if (ttt) sprintf((*ttok).str, "true");
        else (*ttok).str[0] = 0;
        return;
    }
    if (totype == SBOO && now == SFLO){
        union{
            uint32_t y;
            float x;
        }ttt;
        sscanf((*ttok).str, "%f", &ttt.x);
        ttt.y &= 0x7FFFFFFF;
        if (ttt.y) sprintf((*ttok).str, "true");
        else (*ttok).str[0] = 0;
        return;
    }
}

Token doexpr(int head, int tail, int *success){printf("doexpr%d %d\n",head,tail);
    if (head > tail){
        Log("head > tail error: [%d, %d]\n", head, tail);
        *success = 0;
        return tokens[0];
    }
    if (head == tail){
        if (tokens[head].type == DIG){
            int tlen = strlen(tokens[head].str), i;
            for (i = 0; i < tlen; i ++ )
                if (tokens[head].str[i] < '0' || tokens[head].str[i] > '9'){
                    Log("DIG error: %s\n", tokens[head].str);
                    *success = FAIL;
                    return tokens[head];
                }
            *success = SDIG;
            return tokens[head];
        }
        else if (tokens[head].type == HEX){
            int tlen = strlen(tokens[head].str), i;
            if (tlen < 3){
                HEXERR:;
                Log("HEX error: %s\n", tokens[head].str);
                *success = FAIL;
                return tokens[head];
            }
            for (i = 2; i < tlen; i ++ )
                if ((tokens[head].str[i] < '0' || tokens[head].str[i] > '9') &&
                    (tokens[head].str[i] < 'a' || tokens[head].str[i] > 'f') &&
                    (tokens[head].str[i] < 'A' || tokens[head].str[i] > 'F'))
                    goto HEXERR;
            *success = SHEX;
            return tokens[head];
        }
        else if (tokens[head].type == LTR){
            int tlen = strlen(tokens[head].str), i;
            int temp = 0;
            char *s = tokens[head].str;
            if (tlen < 3 || tlen > 6){
                LTRERR:;
                Log("LTR error: %s\n", s);
                *success = FAIL;
                return tokens[head];
            }
            if (tlen == 3){
                if (s[1] == '\\') goto LTRERR;
                temp = s[1];
            }
            else if (s[1] != '\\') goto LTRERR;
            else if (s[2] == 'a') temp = '\a';
            else if (s[2] == 'b') temp = '\b';
            else if (s[2] == 'r') temp = '\r';
            else if (s[2] == 'n') temp = '\n';
            else if (s[2] == 'f') temp = '\f';
            else if (s[2] == 't') temp = '\t';
            else if (s[2] == 'v') temp = '\v';
            else if (s[2] == '\\') temp = '\\';
            else if (s[2] == '\'') temp = '\'';
            else if (s[2] == '\"') temp = '\"';
            else if (s[2] == 'x'){
                temp = 0;
                if (tlen < 5) goto LTRERR;
                for (i = 3; i < tlen - 1; i ++ )
                    if ((s[i] >= '0' && s[i] <= '9') ||
                        (s[i] >= 'a' && s[i] <= 'f') ||
                        (s[i] >= 'A' && s[i] <= 'F')){
                        if (s[i] >= 'A' && s[i] <= 'F') s[i] -= 'A' - '0' - 10;
                        if (s[i] >= 'a' && s[i] <= 'f') s[i] -= 'a' - '0' - 10;
                        s[i] -= '0';
                        temp = temp * 16 + s[i];
                    }
                    else goto LTRERR;
            }
            else{
                temp = 0;
                if (tlen < 4) goto LTRERR;
                for (i = 2; i < tlen - 1; i ++ )
                    if (s[i] < '0' || s[i] > '7') goto LTRERR;
                    else temp = temp * 8 + s[i] - '0';
            }
            sprintf(s, "%d", temp);
            *success = SDIG;
            return tokens[head];
        }
        else if (tokens[head].type == FLOAT){
            *success = SFLO;
            return tokens[head];
        }
        else if (tokens[head].type == REG){
            int tlen = strlen(tokens[head].str), i;
            char *s = tokens[head].str;
            if (tlen != 3 && tlen != 4){
                REGERR:;
                Log("REG error: %s\n", tokens[head].str);
                *success = FAIL;
                return tokens[head];
            }
            for (i = 1; i < tlen; i ++ )
                if (s[i] >= 'A' && s[i] <= 'Z')
                    s[i] -= 'A' - 'a';
            if (tlen == 3){
                /*if (s[1] == 'a' && s[2] == 'x'){
                    sprintf(s, "0x%x", cpu.ax);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 'b' && s[2] == 'x'){
                    sprintf(s, "0x%x", cpu.bx);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 'c' && s[2] == 'x'){
                    sprintf(s, "0x%x", cpu.cx);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 'b' && s[2] == 'p'){
                    sprintf(s, "0x%x", cpu.bp);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 's' && s[2] == 'p'){
                    sprintf(s, "0x%x", cpu.sp);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 's' && s[2] == 'i'){
                    sprintf(s, "0x%x", cpu.si);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 'd' && s[2] == 'i'){
                    sprintf(s, "0x%x", cpu.di);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 'a' && s[2] == 'l'){
                    sprintf(s, "0x%x", cpu.al);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 'b' && s[2] == 'l'){
                    sprintf(s, "0x%x", cpu.bl);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 'c' && s[2] == 'l'){
                    sprintf(s, "0x%x", cpu.cl);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 'd' && s[2] == 'l'){
                    sprintf(s, "0x%x", cpu.dl);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 'd' && s[2] == 'x'){
                    sprintf(s, "0x%x", cpu.dx);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 'a' && s[2] == 'h'){
                    sprintf(s, "0x%x", cpu.ah);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 'b' && s[2] == 'h'){
                    sprintf(s, "0x%x", cpu.bh);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 'c' && s[2] == 'h'){
                    sprintf(s, "0x%x", cpu.ch);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[1] == 'd' && s[2] == 'h'){
                    sprintf(s, "0x%x", cpu.dh);
                    *success = SHEX;
                    return tokens[head];
                }
                else */goto REGERR;
            }
            else{
                if (s[2] == 'a' && s[3] == 'x' && s[1] == 'e'){
                    sprintf(s, "0x%x", cpu.eax);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[2] == 'b' && s[3] == 'x' && s[1] == 'e'){
                    sprintf(s, "0x%x", cpu.ebx);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[2] == 'c' && s[3] == 'x' && s[1] == 'e'){
                    sprintf(s, "0x%x", cpu.ecx);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[2] == 'd' && s[3] == 'x' && s[1] == 'e'){
                    sprintf(s, "0x%x", cpu.edx);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[2] == 'b' && s[3] == 'p' && s[1] == 'e'){
                    sprintf(s, "0x%x", cpu.ebp);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[2] == 's' && s[3] == 'p' && s[1] == 'e'){
                    sprintf(s, "0x%x", cpu.esp);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[2] == 's' && s[3] == 'i' && s[1] == 'e'){
                    sprintf(s, "0x%x", cpu.esi);
                    *success = SHEX;
                    return tokens[head];
                }
                else if (s[2] == 'd' && s[3] == 'i' && s[1] == 'e'){
                    sprintf(s, "0x%x", cpu.edi);
                    *success = SHEX;
                    return tokens[head];
                }
                else goto REGERR;
            }
            *success = SHEX;
            return tokens[head];
        }
        else if (tokens[head].type == VAR){
            if (strcmp(tokens[head].str, "true") == 0){
                *success = SBOO;
                return tokens[head];
            }
            else if (strcmp(tokens[head].str, "false") == 0){
                *success = SBOO;
                tokens[head].str[0] = 0;
                return tokens[head];
            }
            else{
                Log("Now can't calculate variable: %s\n", tokens[head].str);
                *success = 0;
                return tokens[head];
            }
        }
        else{
            Log("1 length error: %s\n", tokens[head].str);
            success = 0;
            return tokens[0];
        }
    }
    int i, left = 0, right = 0;
    for (i = head; i <= tail; i ++ )
        if (prio[i] < prio[left])
            left = right = i;
        else if (prio[i] == prio[right])
            right = i;
    if (!left){
        Log("Too nums and no operator: [%d, %d]\n", head, tail);
        *success = 0;
        return tokens[0];
    }
    if (prio[left] % BRACKET_STEP == 0){//          '(' ')'
        if (left != head || tokens[head].type != '(' ||
            right != tail || tokens[tail].type != ')'){
            Log("Brackets error: [%d, %d]\n", head, tail);
            *success = 0;
            return tokens[0];
        }
        return doexpr(head + 1, tail - 1, success);
    }//             '()' end.
    else if (prio[left] % BRACKET_STEP == 1){//     '?' ':'
        int tmp = 0, i, suc1 = 0, suc2 = 0, suc3 = 0;
        Token part1, part2, part3;
        for (i = left + 1; i <= tail; i ++ )
            if (tokens[i].type == '?') tmp ++ ;
            else if (tokens[i].type == ':'){
                if (!tmp){
                    right = i;
                    break;
                }
                tmp -- ;
            }
        if (tokens[right].type != ':'){
            Log("operator\'?:\' error: [%d, %d]\n", head, tail);
            *success = 0;
            return tokens[0];
        }
        part1 = doexpr(head, left - 1, &suc1);
        if (suc1 == FAIL){
            *success = 0;
            return tokens[0];
        }
        Type_convert(SBOO, &suc1, &part1);
        if (part1.str[0]){
            part2 = doexpr(left + 1, right - 1, &suc2);
            *success = suc2;
            return part2;
        }
        else{
            part3 = doexpr(right + 1, tail, &suc3);
            *success = suc3;
            return part3;
        }
    }//         '?:' end.
    else if (prio[left] % BRACKET_STEP == 2){//     '||'
        int suc1 = 0, suc2 = 0;
        Token step1, step2;
        step1 = doexpr(head, right - 1, &suc1);
        if (suc1 == FAIL){
            *success = FAIL;
            return step1;
        }
        Type_convert(SBOO, &suc1, &step1);
        if (step1.str[0]){
            *success = SBOO;
            return step1;
        }
        step2 = doexpr(right + 1, tail, &suc2);
        Type_convert(SBOO, &suc2, &step2);
        *success = suc2;
        return step2;
    }//            '||' end.
    else if (prio[left] % BRACKET_STEP == 3){//     '&&'
        int suc1 = 0, suc2 = 0;
        Token step1, step2;
        step1 = doexpr(head, right - 1, &suc1);
        if (suc1 == FAIL){
            *success = FAIL;
            return step1;
        }
        Type_convert(SBOO, &suc1, &step1);
        if (!step1.str[0]){
            *success = SBOO;
            return step1;
        }
        step2 = doexpr(right + 1, tail, &suc2);
        Type_convert(SBOO, &suc2, &step2);
        *success = suc2;
        return step2;
    }//             '&&' end.
    else if (prio[left] % BRACKET_STEP == 4){//     '|'
        int suc1, suc2;
        Token step1, step2;
        step1 = doexpr(head, right - 1, &suc1);
        if (suc1 == FAIL){
            _OFAIL:;
            *success = 0;
            return tokens[0];
        }
        if (suc1 == SFLO){
            _OFLO:;
            Log("cannot calculate float with \'|\': [%d, %d]\n", head, tail);
            *success = 0;
            return tokens[0];
        }
        Type_convert(SDIG, &suc1, &step1);
        step2 = doexpr(right + 1, tail, &suc2);
        if (suc2 == FAIL) goto _OFAIL;
        if (suc2 == SFLO) goto _OFLO;
        Type_convert(SDIG, &suc2, &step2);
        int t1, t2;
        sscanf(step1.str, "%d", &t1);
        sscanf(step2.str, "%d", &t2);
        printf("%d %d\n", t1, t2);
        t1 |= t2;
        sprintf(step1.str, "%d", t1);
        *success = SDIG;
        return step1;
    }//             '|' end.
    else if (prio[left] % BRACKET_STEP == 5){//     '^'
        int suc1, suc2;
        Token step1, step2;
        step1 = doexpr(head, right - 1, &suc1);
        if (suc1 == FAIL){
            _XFAIL:;
            *success = 0;
            return tokens[0];
        }
        if (suc1 == SFLO){
            _XFLO:;
            Log("cannot calculate float with \'^\': [%d, %d]\n", head, tail);
            *success = 0;
            return tokens[0];
        }
        Type_convert(SDIG, &suc1, &step1);
        step2 = doexpr(right + 1, tail, &suc2);
        if (suc2 == FAIL) goto _XFAIL;
        if (suc2 == SFLO) goto _XFLO;
        Type_convert(SDIG, &suc2, &step2);
        int t1, t2;
        sscanf(step1.str, "%d", &t1);
        sscanf(step2.str, "%d", &t2);
        printf("%d %d\n", t1, t2);
        t1 ^= t2;
        sprintf(step1.str, "%d", t1);
        *success = SDIG;
        return step1;
    }//             '^' end.
    else if (prio[left] % BRACKET_STEP == 6){//     '&'
        int suc1, suc2;
        Token step1, step2;
        step1 = doexpr(head, right - 1, &suc1);
        if (suc1 == FAIL){
            _AFAIL:;
            *success = 0;
            return tokens[0];
        }
        if (suc1 == SFLO){
            _AFLO:;
            Log("cannot calculate float with \'&\': [%d, %d]\n", head, tail);
            *success = 0;
            return tokens[0];
        }
        Type_convert(SDIG, &suc1, &step1);
        step2 = doexpr(right + 1, tail, &suc2);
        if (suc2 == FAIL) goto _AFAIL;
        if (suc2 == SFLO) goto _AFLO;
        Type_convert(SDIG, &suc2, &step2);
        int t1, t2;
        sscanf(step1.str, "%d", &t1);
        sscanf(step2.str, "%d", &t2);
        printf("%d %d\n", t1, t2);
        t1 &= t2;
        sprintf(step1.str, "%d", t1);
        *success = SDIG;
        return step1;
    }//             '&' end.
    else if (prio[left] % BRACKET_STEP == 7){//     '==' '!='
        int suc1 = 0, suc2 = 0, change = 0;
        if (tokens[right].type == NEQ) change = 1;
        Token step1, step2;
        step1 = doexpr(head, right - 1, &suc1);
        step2 = doexpr(right + 1, tail, &suc2);
        if (suc1 == FAIL || suc2 == FAIL){
            *success = 0;
            return tokens[0];
        }
        int flag = max(suc1, suc2);
        Type_convert(flag, &suc1, &step1);
        Type_convert(flag, &suc2, &step2);
        if (flag == SBOO){
            if (((step1.str[0] == 0) == (step2.str[0] == 0)) ^ change)
                goto SEVEN_TRUE;
            else goto SEVEN_FALSE;
        }
        if (flag == SDIG){
            int t1, t2;
            sscanf(step1.str, "%d", &t1);
            sscanf(step2.str, "%d", &t2);
            if ((t1 == t2) ^ change) goto SEVEN_TRUE;
            else goto SEVEN_FALSE;
        }
        if (flag == SHEX){
            unsigned t1, t2;
            sscanf(step1.str, "%x", &t1);
            sscanf(step2.str, "%x", &t2);
            if ((t1 == t2) ^ change) goto SEVEN_TRUE;
            else goto SEVEN_FALSE;
        }
        if (flag == SFLO){
            float t1, t2;
            sscanf(step1.str, "%f", &t1);
            sscanf(step2.str, "%f", &t2);
            if ((t1 == t2) ^ change) goto SEVEN_TRUE;
            else goto SEVEN_FALSE;
        }
        SEVEN_TRUE:;
        *success = SBOO;
        step1.str[0] = 't';
        return step1;
        SEVEN_FALSE:;
        *success = SBOO;
        step1.str[0] = 0;
        return step1;
    }//             '==' '!=' end.
    else if (prio[left] % BRACKET_STEP == 8){//     '>' '>=' '<' '<='
        int suc1 = 0, suc2 = 0, change = 0, equal = 0;
        if (tokens[right].type == '<' ||
            tokens[right].type == SEQ) change = 1;
        if (tokens[right].type == '<' ||
            tokens[right].type == BEQ) equal = 1;
        Token step1, step2;
        step1 = doexpr(head, right - 1, &suc1);
        step2 = doexpr(right + 1, tail, &suc2);
        if (suc1 == FAIL || suc2 == FAIL){
            *success = 0;
            return tokens[0];
        }
        int flag = max(suc1, suc2);
        Type_convert(flag, &suc1, &step1);
        Type_convert(flag, &suc2, &step2);
        if (flag == SBOO){
            int t1 = !!step1.str[0], t2 = !!step1.str[0];
            if (((t1 > t2) || (equal && (t1 == t2))) ^ change)
                goto EIGHT_TRUE;
            else goto EIGHT_FALSE;
        }
        if (flag == SDIG){
            int t1, t2;
            sscanf(step1.str, "%d", &t1);
            sscanf(step2.str, "%d", &t2);
            if (((t1 > t2) || (equal && (t1 == t2))) ^ change)
                goto EIGHT_TRUE;
            else goto EIGHT_FALSE;
        }
        if (flag == SHEX){
            unsigned t1, t2;
            sscanf(step1.str, "%x", &t1);
            sscanf(step2.str, "%x", &t2);
            if (((t1 > t2) || (equal && (t1 == t2))) ^ change)
                goto EIGHT_TRUE;
            else goto EIGHT_FALSE;
        }
        if (flag == SFLO){
            float t1, t2;
            sscanf(step1.str, "%f", &t1);
            sscanf(step2.str, "%f", &t2);
            if (((t1 > t2) || (equal && (t1 == t2))) ^ change)
                goto EIGHT_TRUE;
            else goto EIGHT_FALSE;
        }
        EIGHT_TRUE:;
        *success = SBOO;
        step1.str[0] = 't';
        return step1;
        EIGHT_FALSE:;
        *success = SBOO;
        step1.str[0] = 0;
        return step1;
    }//             '>' '>=' '<' '<='end.
    else if (prio[left] % BRACKET_STEP == 9){//     '>>' '<<'
        int suc1, suc2;
        Token step1, step2;
        step1 = doexpr(head, right - 1, &suc1);
        if (suc1 == FAIL){
            _LLRRFAIL:;
            *success = 0;
            return tokens[0];
        }
        if (suc1 == SFLO){
            _LLRRFLO:;
            Log("cannot calculate float with \'<<\' or \'>>\': [%d, %d]\n", head, tail);
            *success = 0;
            return tokens[0];
        }
        step2 = doexpr(right + 1, tail, &suc2);
        if (suc2 == FAIL) goto _LLRRFAIL;
        if (suc2 == SFLO) goto _LLRRFLO;
        int flag = max(suc1, suc2);
        if (flag == SBOO) flag = SDIG;
        Type_convert(flag, &suc1, &step1);
        Type_convert(flag, &suc2, &step2);
        if (flag == SDIG){
            int t1, t2;
            sscanf(step1.str, "%d", &t1);
            sscanf(step2.str, "%d", &t2);
            //printf("%d %d\n", t1, t2);
            if (tokens[right].type == LL) t1 <<= t2;
            else t1 >>= t2;
            sprintf(step1.str, "%d", t1);
            *success = SDIG;
            return step1;
        }
        else{
            uint32_t t1, t2;
            sscanf(step1.str, "%x", &t1);
            sscanf(step2.str, "%x", &t2);
            //printf("%d %d\n", t1, t2);
            if (tokens[right].type == LL) t1 <<= t2;
            else t1 >>= t2;
            sprintf(step1.str, "%x", t1);
            *success = SHEX;
            return step1;
        }
    }//             '<<' '>>' end.
    else if (prio[left] % BRACKET_STEP == 10){//    '+' '-'
        int suc1 = 0, suc2 = 0;
        Token step1, step2;
        step1 = doexpr(head, right - 1, &suc1);
        step2 = doexpr(right + 1, tail, &suc2);
        if (suc1 == FAIL || suc2 == FAIL){
            *success = 0;
            return tokens[0];
        }
        int flag = max(suc1, suc2);
        if (flag == SBOO) flag = SDIG;
        Type_convert(flag, &suc1, &step1);
        Type_convert(flag, &suc2, &step2);
        if (flag == SDIG){
            int t1, t2;
            sscanf(step1.str, "%d", &t1);
            sscanf(step2.str, "%d", &t2);
            if (tokens[right].type == '+') t1 += t2;
            else t1 -= t2;
            sprintf(step1.str, "%d", t1);
            *success = SDIG;
            return step1;
        }
        else if (flag == SHEX){
            uint32_t t1, t2;
            sscanf(step1.str, "%x", &t1);
            sscanf(step2.str, "%x", &t2);
            if (tokens[right].type == '+') t1 += t2;
            else t1 -= t2;
            sprintf(step1.str, "0x%x", t1);
            *success = SHEX;
            return step1;
        }
        else if (flag == SFLO){
            float t1, t2;
            sscanf(step1.str, "%f", &t1);
            sscanf(step2.str, "%f", &t2);
            if (tokens[right].type == '+') t1 += t2;
            else t1 -= t2;
            sprintf(step1.str, "%.20e", t1);
            *success = SFLO;
            return step1;
        }
        *success = 0;
        return tokens[0];
    }//             '+' '-' end.
    else if (prio[left] % BRACKET_STEP == 11){//    '*' '/' '%'
        int suc1 = 0, suc2 = 0;
        Token step1, step2;
        step1 = doexpr(head, right - 1, &suc1);
        step2 = doexpr(right + 1, tail, &suc2);
        if (suc1 == FAIL || suc2 == FAIL){
            *success = 0;
            return tokens[0];
        }
        int flag = max(suc1, suc2);
        if (flag == SBOO) flag = SDIG;
        Type_convert(flag, &suc1, &step1);
        Type_convert(flag, &suc2, &step2);
        if (flag == SDIG){
            int t1, t2;
            sscanf(step1.str, "%d", &t1);
            sscanf(step2.str, "%d", &t2);
            if (tokens[right].type == '*') t1 *= t2;
            else{
                if (t2 == 0){
                    DIV0:;
                    Log("Divide by zero: [%d, %d]\n", head, tail);
                    *success = 0;
                    return tokens[0];
                }
                if (tokens[right].type == '/') t1 /= t2;
                else t1 %= t2;
            }
            sprintf(step1.str, "%d", t1);
            *success = SDIG;
            return step1;
        }
        else if (flag == SHEX){
            uint32_t t1, t2;
            sscanf(step1.str, "%x", &t1);
            sscanf(step2.str, "%x", &t2);
            if (tokens[right].type == '*') t1 *= t2;
            else{
                if (t2 == 0) goto DIV0;
                if (tokens[right].type == '/') t1 /= t2;
                else t1 %= t2;
            }
            sprintf(step1.str, "0x%x", t1);
            *success = SHEX;
            return step1;
        }
        else if (flag == SFLO){
            float t1, t2;
            sscanf(step1.str, "%f", &t1);
            sscanf(step2.str, "%f", &t2);
            if (tokens[right].type == '*') t1 *= t2;
            else if (tokens[right].type == '/') t1 /= t2;
            else{
                Log("Cannot calculate float with \'%%\': [%d , %d]\n", head, tail);
                *success = 0;
                return tokens[0];
            }
            sprintf(step1.str, "%.20e", t1);
            *success = SFLO;
            return step1;
        }
        *success = 0;
        return tokens[0];
    }//             '*' '/' '%' end.
    else if (prio[left] % BRACKET_STEP == 12){//    '+' '-' '*' '!' '~' '&'
        int suc = 0;
        Token step;
        step = doexpr(left + 1, tail, &suc);
        if (suc == FAIL){
            *success = 0;
            return tokens[0];
        }
        if (tokens[left].type == '+'){
            if (suc == SBOO){
                Type_convert(SDIG, &suc, &step);
                *success = suc;
                return step;
            }
            *success = suc;
            return step;
        }
        else if (tokens[left].type == '-'){
            if (suc == SBOO)
                Type_convert(SDIG, &suc, &step);
            if (suc == SDIG){
                int t;
                sscanf(step.str, "%d", &t);
                sprintf(step.str, "%d", t * - 1);
                *success = suc;
                return step;
            }
            else if (suc == SHEX){
                uint32_t t;
                sscanf(step.str, "%x", &t);
                sprintf(step.str, "0x%x", t * - 1);
                *success = suc;
                return step;
            }
            else{
                float t;
                sscanf(step.str, "%f", &t);
                sprintf(step.str, "%.20e", t * - 1);
                *success = suc;
                return step;
            }
        }
        else if (tokens[left].type == '!'){
            if (suc == SBOO){
                step.str[0] ^= 't';
                *success = suc;
                return step;
            }
            else{
                union{
                    int x;
                    uint32_t y;
                    float z;
                }t;
                if (suc == SDIG) sscanf(step.str, "%d", &t.x);
                else if (suc == SHEX) sscanf(step.str, "%x", &t.y);
                else if (suc == SFLO) sscanf(step.str, "%f", &t.z);
                sprintf(step.str, "%d", !t.x);
                *success = SDIG;
                return step;
            }
        }
        else if (tokens[left].type == '~'){
            if (suc == SBOO)
                Type_convert(SDIG, &suc, &step);
            if (suc == SDIG){
                int t;
                sscanf(step.str, "%d", &t);
                sprintf(step.str, "%d", ~t);
                *success = suc;
                return step;
            }
            else if (suc == SHEX){
                uint32_t t;
                sscanf(step.str, "%x", &t);
                sprintf(step.str, "0x%x", ~t);
                *success = suc;
                return step;
            }
            else{
                Log("cannot calculate float with \'~\': [%d, %d]", head, tail);
                *success = 0;
                return tokens[0];
            }
        }
        else{
            Log("Unknown unary operator: [%d, %d]\n", head, tail);
            *success = 0;
            return tokens[0];
        }

    }
    else{
        printf("more to do\n");
        *success = 0;
        return tokens[0];
    }
}

uint32_t expr(char *e, int *success) {

    /*/
    for (;;){
        int t1, t2;
        Token t3;
        int gua;
        gua = scanf("%d%d%s", &t1, &t2, t3.str);
        gua = 1;
        Type_convert(t1, &t2, &t3);
        printf("%s %d\n", t3.str, gua);
    }
    /*/

	if(!make_token(e)) {
		*success = 0;
		return 13452;
	}
	if (!nr_token){
        *success = - 1;
        return 5464;
	}
	//*success = 1;
	//return 0;

    Token ans = doexpr(1, nr_token, success);
    if (*success == 0) return 0;
    else if (*success == SDIG){
        int tmp;
        sscanf(ans.str, "%d", &tmp);
        return (uint32_t)tmp;
    }
    else if (*success == SHEX){
        uint32_t tmp;
        sscanf(ans.str, "%x", &tmp);
        return tmp;
    }
    else if (*success == SFLO){
        union{
            float x;
            uint32_t y;
        }tmp;
        sscanf(ans.str, "%f", &tmp.x);
        return tmp.y;
    }
    else if (*success == SBOO){
        if (ans.str[0]) return 1;
        else return 0;
    }
    else{
        printf("dididididididididididbabababababababababa\n");
        return 34535;
    }

	/* TODO: Insert codes to evaluate the expression. */
	//panic("please implement me");
}

