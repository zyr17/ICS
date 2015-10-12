#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_list[NR_WP];
static WP *head, *free_;

void init_wp_list() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_list[i].NO = i;
		wp_list[i].next = &wp_list[i + 1];
	}
	wp_list[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_list;
}

WP* new_wp(){
    if (free_ == NULL){
        Log("No enough space for a new watchpoint.");
        return NULL;
    }
    WP *re = free_;
    free_ = (*free_).next;
    (*re).next = head;
    return head = re;
}

void free_wp(WP *wp){
    if (wp == head){
        head = (*head).next;
        return;
    }
    WP *now = head;
    for (; (*now).next != wp; now = (*now).next);
    (*now).next = (*(*now).next).next;
    (*wp).next = free_;
    free_ = wp;
}

WP *find_wp(int k){
    WP *now = head;
    for (; now != NULL && (*now).NO != k; now = (*now).next);
    return now;
}

void output_wp(){
    int tot = 0;
    WP *i;
    for (i = head; i != NULL; tot ++ , i = (*i).next);
    printf("There's %d watchpoint(s).\n\nNO\tExpr\n", tot);
    for (i = head; i != NULL; i = (*i).next)
        printf("%d\t%s\n", (*i).NO, (*i).e);
}

/* TODO: Implement the functionality of watchpoint */


