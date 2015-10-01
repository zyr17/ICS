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
    {"([1234567890]+\\.|[1234567890]+\\.[1234567890]+|\\.[1234567890]+)([eE][1234567890]*){0,1}", FLOAT},
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

typedef struct token {
	int type;
	char str[TOKEN_LEN];
} Token;

Token tokens[TOKEN_TOT];
int nr_token;

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

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
                memcpy(tokens[nr_token].str, substr_start, substr_len);
                tokens[nr_token].str[substr_len] = 0;
                tokens[nr_token].type = rules[i].token_type;

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
#define SDIG 1
#define SHEX 2
#define SFLO 3
#define SBOO 4

Token doexpr(int head, int tail, int *success){printf("doexpr%d %d\n",head,tail);
    if (head > tail){
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
            if (tlen == 3) temp = s[1];
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
                        if (s[i] >= 'A' && s[i] <= 'F') s[i] -= 'A' + '0' + 10;
                        if (s[i] >= 'a' && s[i] <= 'f') s[i] -= 'a' + '0' + 10;
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
    printf("more to do\n");
    *success = 0;
    return tokens[0];
}

uint32_t expr(char *e, int *success) {
	if(!make_token(e)) {
		*success = 0;
		return 0;
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

