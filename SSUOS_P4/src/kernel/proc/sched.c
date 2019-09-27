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
	struct process *next = &procs[0];
	struct list_elem *elem;
	int i, j;
	bool isFirst, isPrint;

	/* 
	   You shoud modify this function...
	   Browse the 'runq' array 
	 */
	found = false;
	for(i=0; i<RQ_NQS; i++){
		if(list_size(&runq[i]) > 0){
			found = true;
			break;
		}
	}
	//
	if(!found){
		//printk("not found %ld\n", get_ticks());
	}
	//
	if(found){
		isFirst = true;
		for(j=1; j<PROC_NUM_MAX; j++){
			if(procs[j].state == PROC_RUN){
				if(!isFirst)
					printk(", ");
				else
					isFirst = false;
				printk("#= %d p= %3d ", 
						procs[j].pid, procs[j].priority);
				printk("c= %3d u= %3d",
						procs[j].time_slice, procs[j].time_used);
			}
		}
		printk("\n");
		for(elem = list_begin(&runq[i]); elem != list_end(&runq[i]); elem = list_next(elem))
		{
			struct process *p = list_entry(elem, struct process, elem_stat);
			if(p->state == PROC_RUN){
				printk("Selected : %d\n", p->pid);
				list_remove(elem);
				return p;
			}
		}
	}
	return next;
}

void schedule(void)
{
	struct process *cur;
	struct process *next;

	/* You shoud modify this function.... */
	proc_wake();
	if(cur_process == idle_process)
		next = get_next_proc();
	else
		next = idle_process;
	cur = cur_process;
	cur_process = next;
	cur_process->time_slice = 0;
	intr_disable();
	switch_process(cur, next);
	intr_enable();
}
