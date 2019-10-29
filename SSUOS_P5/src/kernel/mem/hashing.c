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

int level_insert(int fs, uint32_t idx, uint32_t key, uint32_t value){
	int si = getSlotNum(fs, idx);
	if(si == -1)
		return -1;
	if(fs == 0){
		lh.top_buckets[idx].slot[si].key = key;
		lh.top_buckets[idx].slot[si].value = value;
		lh.top_buckets[idx].token[si] = 1;
		printk("hash value inserted in top level : idx : %d, key : %d, value : %d\n", idx, key, value);
		return 0;
	}
	else{
		lh.bottom_buckets[idx/2].slot[si].key = key;
		lh.bottom_buckets[idx/2].slot[si].value = value;
		lh.bottom_buckets[idx/2].token[si] = 1;
		printk("hash value inserted in bottom level : idx : %d, key : %d, value : %d\n", idx, key, value);
		return 0;

	}
}

int getSlotNum(int fs, uint32_t idx){
	int i=0;
	if(fs == 0){
		for(i=0; i<SLOT_NUM; i++){
			if(lh.top_buckets[idx].token[i] == 0)
				return i;
		}
	}
	else{
		for(i=0; i<SLOT_NUM; i++){
			if(lh.bottom_buckets[idx/2].token[i] == 0)
				return i;
		}
	}
	return -1;
}
