#include <list.h>
#include <proc/sched.h>
#include <mem/malloc.h>
#include <proc/proc.h>
#include <proc/switch.h>
#include <interrupt.h>

extern struct list plist;
extern struct list rlist;
extern struct list runq[RQ_NQS];

extern struct process procs[PROC_NUM_MAX];
extern struct process *idle_process;
struct process *latest;

bool more_prio(const struct list_elem *a, const struct list_elem *b,void *aux);
int scheduling; 					// interrupt.c

struct process* get_next_proc(void) 
{
	bool found = false;
	struct process *next = NULL;
	struct list_elem *elem;

	/* 
	   You shoud modify this function...
	   Browse the 'runq' array 
	*/

	for(elem = list_begin(&rlist); elem != list_end(&rlist); elem = list_next(elem))
	{
		struct process *p = list_entry(elem, struct process, elem_stat);

		if(p->state == PROC_RUN)
			return p;
	}

	return next;
}

void schedule(void)
{
	struct process *cur;
	struct process *next;

	/* You shoud modify this function.... */

	proc_wake();

	next = get_next_proc();
	cur = cur_process;
	cur_process = next;
	cur_process->time_slice = 0;

	switch_process(cur, next);
}
