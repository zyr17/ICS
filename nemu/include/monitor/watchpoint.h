#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"
#define __WATCHPOINT_CLEN__ 1111

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;
	char e[__WATCHPOINT_CLEN__];

	/* TODO: Add more members if necessary */


} WP;

WP* new_wp();
void free_wp(WP *wp);
WP *find_wp(int k);
void output_wp();

#endif
