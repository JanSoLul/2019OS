#include <device/io.h>
#include <mem/mm.h>
#include <mem/paging.h>
#include <device/console.h>
#include <proc/proc.h>
#include <interrupt.h>
#include <mem/palloc.h>
#include <ssulib.h>
#include <mem/hashing.h>

uint32_t F_IDX(uint32_t addr, uint32_t capacity) {
    return addr % ((capacity / 2) - 1);
}

uint32_t S_IDX(uint32_t addr, uint32_t capacity) {
    return (addr * 7) % ((capacity / 2) - 1) + capacity / 2;
}

void init_hash_table(void)
{
	// TODO : OS_P5 assignment
	int i, j;
	struct level_hash lh;
	
	for(i=0; i<CAPACITY; i++){
		for(j=0; j<SLOT_NUM; j++){
			lh.top_buckets[i].token[j] = 0;
			lh.top_buckets[i].slot[j].key = 0;
			lh.top_buckets[i].slot[j].value = 0;
			if(i%2 == 0){
				lh.bottom_buckets[i/2].token[j] = 0;
				lh.bottom_buckets[i/2].slot[j].key = 0;
				lh.bottom_buckets[i/2].slot[j].value = 0;
			}
		}
	}
}
