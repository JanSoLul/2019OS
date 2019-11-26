#include <mem/palloc.h>
#include <bitmap.h>
#include <type.h>
#include <round.h>
#include <mem/mm.h>
#include <synch.h>
#include <device/console.h>
#include <mem/paging.h>
#include<mem/swap.h>

/* Page allocator.  Hands out memory in page-size (or
   page-multiple) chunks.  
   */
/* pool for memory */
struct memory_pool
{
	struct lock lock;                   
	struct bitmap *bitmap; 
	uint32_t *addr;                    
};
/* kernel heap page struct */
struct khpage{
	uint16_t page_type;
	uint16_t nalloc;
	uint32_t used_bit[4];
	struct khpage *next;
};

/* free list */
struct freelist{
	struct khpage *list;
	int nfree;
};

static struct khpage *khpage_list;
static struct freelist freelist;
static uint32_t page_alloc_index;
static struct memory_pool user_pool;
static struct memory_pool kernel_pool;

/* Initializes the page allocator. */
//
	void
init_palloc (void) 
{
	size_t kernel_pages, user_pages;
	size_t bm_pages;

	kernel_pages = (USER_POOL_START - KERNEL_ADDR) / PAGE_SIZE;
	bm_pages = DIV_ROUND_UP(bitmap_struct_size(kernel_pages), PAGE_SIZE);
	kernel_pages -= bm_pages;
	lock_init(&kernel_pool.lock);
	kernel_pool.bitmap = create_bitmap(kernel_pages, (uint32_t *)KERNEL_ADDR, bm_pages * PAGE_SIZE);
	kernel_pool.addr = (uint32_t *)(KERNEL_ADDR + bm_pages * PAGE_SIZE);
	printk("kernel ptr : %x\n", kernel_pool.addr);

	user_pages = (RKERNEL_HEAP_START - USER_POOL_START) / PAGE_SIZE;
	bm_pages = DIV_ROUND_UP(bitmap_struct_size(user_pages), PAGE_SIZE);
	user_pages -= bm_pages;
	lock_init(&user_pool.lock);
	user_pool.bitmap = create_bitmap(user_pages, (uint32_t *)USER_POOL_START, bm_pages * PAGE_SIZE);
	user_pool.addr = (uint32_t *)(USER_POOL_START + bm_pages * PAGE_SIZE);
	printk("user ptr : %x\n", user_pool.addr);
}

/* Obtains and returns a group of PAGE_CNT contiguous free pages.
   */
	uint32_t *
palloc_get_multiple_page (enum palloc_flags flags, size_t page_cnt)
{
	void *pages = NULL;
	size_t page_idx;
	struct memory_pool *target_pool = NULL;

	if(flags & kernel_area)
		target_pool = &kernel_pool;
	else
		target_pool = &user_pool;

	if (page_cnt == 0)
		return NULL;

	lock_acquire(&target_pool->lock);
	page_idx = find_set_bitmap(target_pool->bitmap, 0, page_cnt, false);
	lock_release(&target_pool->lock);
	printk("idx : %d\n", page_idx);

	if(page_idx != BITMAP_ERROR)
		pages = target_pool->addr + PAGE_SIZE * page_idx;
	else
		pages = NULL;

	if(pages != NULL)
		memset(pages, 0, PAGE_SIZE * page_cnt);
	else
		return NULL;

	return (uint32_t *)pages;
}

/* Obtains a single free page and returns its address.
   */
	uint32_t *
palloc_get_one_page (enum palloc_flags flags) 
{
	return palloc_get_multiple_page (flags, 1);
}

/* Frees the PAGE_CNT pages starting at PAGES. */
	void
palloc_free_multiple_page (void *pages, size_t page_cnt) 
{
	struct memory_pool *target_pool;
	size_t page_idx;


	if(pages == NULL || page_cnt == 0)
		return;

	if(KERNEL_ADDR <= (uint32_t)pages && (uint32_t)pages < USER_POOL_START)
		target_pool = &kernel_pool;
	else
		target_pool = &user_pool;

	page_idx = (uint32_t)((uint32_t*)pages - target_pool->addr) / PAGE_SIZE;

	bitmap_all(target_pool->bitmap, page_idx, page_cnt);
	set_multi_bitmap(target_pool->bitmap, page_idx, page_cnt, false);

	/*
	struct khpage *khpage = freelist.list;
	size_t page_idx = (((uint32_t)pages - VKERNEL_HEAP_START) / PAGE_SIZE);

	if (pages == NULL || page_cnt == 0)
		return;

	if(khpage == NULL){
		freelist.list = khpage_list + page_idx;
		freelist.list->nalloc = page_cnt;
		freelist.list->next = NULL;
	}
	else{

		while(khpage->next != NULL){
			khpage = khpage->next;
		}

		khpage->next = khpage_list + page_idx;
		khpage->next->nalloc = page_cnt;
		khpage->next->next = NULL;
	}

	freelist.nfree++;
	*/
}

/* Frees the page at PAGE. */
	void
palloc_free_one_page (void *page) 
{
	palloc_free_multiple_page (page, 1);
}


void palloc_pf_test(void)
{
	uint32_t *one_page1 = palloc_get_one_page(user_area);
	uint32_t *one_page2 = palloc_get_one_page(user_area);
	uint32_t *two_page1 = palloc_get_multiple_page(user_area,2);
	uint32_t *three_page;
	printk("one_page1 = %x\n", one_page1); 
	printk("one_page2 = %x\n", one_page2); 
	printk("two_page1 = %x\n", two_page1);

	printk("=----------------------------------=\n");
	palloc_free_one_page(one_page1);
	palloc_free_one_page(one_page2);
	palloc_free_multiple_page(two_page1,2);

	one_page1 = palloc_get_one_page(user_area);
	one_page2 = palloc_get_one_page(user_area);
	two_page1 = palloc_get_multiple_page(user_area,2);

	printk("one_page1 = %x\n", one_page1);
	printk("one_page2 = %x\n", one_page2);
	printk("two_page1 = %x\n", two_page1);

	printk("=----------------------------------=\n");
	palloc_free_multiple_page(one_page2, 3);
	one_page2 = palloc_get_one_page(user_area);
	three_page = palloc_get_multiple_page(user_area,3);

	printk("one_page1 = %x\n", one_page1);
	printk("one_page2 = %x\n", one_page2);
	printk("three_page = %x\n", three_page);

	palloc_free_one_page(one_page1);
	palloc_free_one_page(three_page);
	three_page = (uint32_t*)((uint32_t)three_page + 0x1000);
	palloc_free_one_page(three_page);
	three_page = (uint32_t*)((uint32_t)three_page + 0x1000);
	palloc_free_one_page(three_page);
	palloc_free_one_page(one_page2);
}
