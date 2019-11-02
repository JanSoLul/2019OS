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
	for(i=0; i<CAPACITY; i++){
		for(j=0; j<SLOT_NUM; j++){
			hash_table.top_buckets[i].token[j] = 0;
			hash_table.top_buckets[i].slot[j].key = 0;
			hash_table.top_buckets[i].slot[j].value = 0;
			if(i%2 == 0){
				hash_table.bottom_buckets[i/2].token[j] = 0;
				hash_table.bottom_buckets[i/2].slot[j].key = 0;
				hash_table.bottom_buckets[i/2].slot[j].value = 0;
			}
		}
	}
}

int level_insert(int tb, uint32_t idx, uint32_t key, uint32_t value){
	int si = get_slot_num(tb, idx);
	if(si == -1)
		return -1;
	if(tb == 0){
		hash_table.top_buckets[idx].slot[si].key = key;
		hash_table.top_buckets[idx].slot[si].value = value;
		hash_table.top_buckets[idx].token[si] = 1;
		printk("hash value inserted in top level : idx : %d, key : %d, value : %x\n", idx, key, value);
		return 0;
	}
	else{
		hash_table.bottom_buckets[idx/2].slot[si].key = key;
		hash_table.bottom_buckets[idx/2].slot[si].value = value;
		hash_table.bottom_buckets[idx/2].token[si] = 1;
		printk("hash value inserted in bottom level : idx : %d, key : %d, value : %x\n", idx, key, value);
		return 0;

	}
}

int get_slot_num(int tb, uint32_t idx){
	int i=0;
	if(tb == 0){
		for(i=0; i<SLOT_NUM; i++){
			if(hash_table.top_buckets[idx].token[i] == 0)
				return i;
		}
	}
	else{
		for(i=0; i<SLOT_NUM; i++){
			if(hash_table.bottom_buckets[idx/2].token[i] == 0)
				return i;
		}
	}
	return -1;
}

int move_bucket(int tb, uint32_t idx, uint32_t value){
	int i=0;
	int si;
	int tmp;
	int f_idx, s_idx;

	if(tb == 0){
		tmp = f_idx;
		if((si = get_slot_num(tb, f_idx)) == -1){
			tmp = s_idx;
			if((si = get_slot_num(tb, s_idx)) == -1){
				return -1;
			}
		}
		hash_table.top_buckets[tmp].token[si] = 1;
		hash_table.top_buckets[tmp].slot[si].key = hash_table.top_buckets[idx].slot[0].key;
		hash_table.top_buckets[tmp].slot[si].value = hash_table.top_buckets[idx].slot[0].value;
		hash_table.top_buckets[idx].token[0] = 0;

	}
	else{
		tmp = f_idx/2;
		if((si = get_slot_num(tb, f_idx)) == -1){
			tmp = s_idx/2;
			if((si = get_slot_num(tb, s_idx)) == -1){
				return -1;
			}
		}
		hash_table.bottom_buckets[tmp].token[si] = 1;
		hash_table.bottom_buckets[tmp].slot[si].key = hash_table.bottom_buckets[idx/2].slot[0].key;
		hash_table.bottom_buckets[tmp].slot[si].value = hash_table.bottom_buckets[idx/2].slot[0].value;
		hash_table.bottom_buckets[idx/2].token[0] = 0;
	}
	return 0;
}

int level_delete(uint32_t f_idx, uint32_t s_idx, uint32_t key, uint32_t value){
	int i, j;
	uint32_t idx_list[2] = {f_idx, s_idx};
	for(i=0; i<2; i++){
		for(j=0; j<SLOT_NUM; j++){
			if(hash_table.top_buckets[idx_list[i]].token[j] == 1
				&& hash_table.top_buckets[idx_list[i]].slot[j].key == key){
				hash_table.top_buckets[idx_list[i]].token[j] = 0;
				printk("hash value deleted : idx : %d, key : %d, value : %x\n", idx_list[i], key, value);
				return 0;
			}
			if(hash_table.bottom_buckets[idx_list[i]/2].token[j] == 1
				&& hash_table.bottom_buckets[idx_list[i]/2].slot[j].key == key){
				hash_table.bottom_buckets[idx_list[i]/2].token[j] = 0;
				printk("hash value deleted : idx : %d, key : %d, value : %x\n", idx_list[i]/2, key, value);
				return 0;
			}
		}
	}
	return -1;
}
