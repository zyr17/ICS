#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"
#define __WATCHPOINT_CLEN__ 1111

typedef struct watchpoint {
	int NO, success;
	struct watchpoint *next;
	char e[__WATCHPOINT_CLEN__];
	union{
        int a;
        uint32_t b;
        float c;
	}number;

	/* TODO: Add more members if necessary */


} WP;

WP* new_wp();
void free_wp(WP *wp);
WP *find_wp(int k);
void output_wp();
bool wp_is_change();

#endif
