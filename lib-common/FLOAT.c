#include "FLOAT.h"

FLOAT F_mul_F(FLOAT a, FLOAT b) {
    int tmp = (int)a * b;
	return (int)(tmp / 0x10000);
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
    int tmp = a;
    tmp *= 0x10000;
    tmp /= b;
	return (int)tmp;
}

FLOAT f2F(float a) {
	return (int)(a * 0x10000);
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

