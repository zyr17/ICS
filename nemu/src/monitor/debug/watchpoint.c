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
        WP *tmp = (*head).next;
        (*head).next = free_;
        free_ = head;
        head = tmp;
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
    printf("There's %d watchpoint(s).\n", tot);
    if (!tot) return;
    printf("\nNO\tExpr\n");
    for (i = head; i != NULL; i = (*i).next)
        printf("%d\t%s\n", (*i).NO, (*i).e);
}

bool is_change(){
    bool re = 0;
    WP *i = head;
    for (; i != NULL; i = (*i).next){
        int suc = 0;
        uint32_t tnum = expr((*i).e, &suc);
        if (suc == FAIL){
            printf("An error occuredon calculating Expr \"%s\", NO %d.\n", (*i).e, (*i).NO);
            re = 1;
        }
        else if (suc == SDIG ||suc == SHEX || suc == SFLO || suc == SBOO){
            if (tnum != (*i).number.b){
                printf("Watchpoint NO %d has changed from ", (*i).NO);
                if (suc == SDIG){
                    printf("%d to ", (*i).number.a);
                    (*i).number.b = tnum;
                    printf("%d.\n", (*i).number.a);
                    (*i).success = suc;
                }
                else if (suc == SHEX){
                    printf("0x%X to ", (*i).number.b);
                    (*i).number.b = tnum;
                    printf("0x%X.\n", (*i).number.b);
                    (*i).success = suc;
                }
                else if (suc == SFLO){
                    printf("%f to ", (*i).number.c);
                    (*i).number.b = tnum;
                    printf("%f.\n", (*i).number.c);
                    (*i).success = suc;
                }
                else{
                    if ((*i).number.b) printf("true to false.\n");
                    else printf("false to true.\n");
                    (*i).number.b = tnum;
                    (*i).success = suc;
                }
            }
        }
    }
    return re;
}

/* TODO: Implement the functionality of watchpoint */


