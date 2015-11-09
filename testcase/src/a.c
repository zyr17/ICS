#include "trap.h"
char str[] = "123456789adsgfbre";
int func(int k){
	if (!k) return 0;
	return func(k - 1) + 1;
}
int main(){
	if (func(100) == 99) HIT_GOOD_TRAP;
	HIT_BAD_TRAP;
	return 0;
}
