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
    {"[1234567890]*\\.[1234567890]*|[1234567890]*\\.{0,1}[1234567890]*[eE][1234567890]+", FLOAT},
    {"0[xX][\\dabcdefABCDEF]+", HEX},
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

//success:   1:dig  2:hex  3:float  4:bool

/*Token doexpr(int head, int tail, int *success){
    if (head > tail){
        *success = 0;
        return Token[0];
    }
    if (head == tail){
        if (tokens[head].type == DIG){

        }
        else if (tokens[head].type == HEX){

        }
        else if (tokens[head].type == LTR){

        }
        else if (tokens[head].type == FLOAT){

        }
        else if (tokens[head].type == REG){

        }
        else if (tokens[head].type == VAR){

        }
        else{
            success = 0;
            return tokens[0];
        }
    }
}*/

uint32_t expr(char *e, int *success) {
	if(!make_token(e)) {
		*success = 0;
		return 0;
	}
	*success = 1;
	return 0;

    //Token ans = doexpr(0, nr_token, success);
    //if (!success) return 0;

	/* TODO: Insert codes to evaluate the expression. */
	//panic("please implement me");
	//return ans;
}

