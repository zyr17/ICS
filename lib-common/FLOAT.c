#include "FLOAT.h"

FLOAT F_mul_F(FLOAT a, FLOAT b) {
    long long tmp = (long long)a * b;
	return (int)(tmp >> 16);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
    if (!(b & 0x7fffffff)) for (;;);
    int fh = (a < 0 ? - 1 : 1) * (b < 0 ? - 1 : 1);
    if (a < 0) a *= - 1;
    if (b < 0) b *= - 1;
    int ans = a / b, i = 16;
    a %= b;
    for (; i -- ; ){
        ans *= 2;
        a *= 2;
        if (a >= b){
            ans ++ ;
            a -= b;
        }
    }
	return ans;
}

FLOAT f2F(float a) {
    int ans = 0, mul = 0;
    union{
        float x;
        unsigned y;
    }pp;
    pp.x = a;
    mul = (pp.y >> 24) & 0x7f;
    mul -= 127;
    ans = pp.y & 0xffffff;
    ans <<= mul;
	return ans * (pp.y >> 31 ? - 1 : 1);
}

FLOAT Fabs(FLOAT a) {
	return a & 0x7fffffff;
}

FLOAT sqrt(FLOAT x) {
	FLOAT dt, t = int2F(2);

	do {
		dt = F_div_int((F_div_F(x, t) - t), 2);
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

FLOAT pow(FLOAT x, FLOAT y) {
	/* we only compute x^0.333 */
	FLOAT t2, dt, t = int2F(2);

	do {
		t2 = F_mul_F(t, t);
		dt = (F_div_F(x, t2) - t) / 3;
		t += dt;
	} while(Fabs(dt) > f2F(1e-4));

	return t;
}

