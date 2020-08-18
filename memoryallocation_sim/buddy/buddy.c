/**
 * Buddy Allocator
 *
 * For the list library usage, see http://www.mcs.anl.gov/~kazutomo/list/
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/
#define USE_DEBUG 0

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "buddy.h"
#include "list.h"

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define MIN_ORDER 12
#define MAX_ORDER 20

#define PAGE_SIZE (1<<MIN_ORDER)
/* page index to address */
#define PAGE_TO_ADDR(page_idx) (void *)((page_idx*PAGE_SIZE) + g_memory)

/* address to page index */
#define ADDR_TO_PAGE(addr) ((unsigned long)((void *)addr - (void *)g_memory) / PAGE_SIZE)

/* find buddy address */
#define BUDDY_ADDR(addr, o) (void *)((((unsigned long)addr - (unsigned long)g_memory) ^ (1<<o)) \
									 + (unsigned long)g_memory)

#if USE_DEBUG == 1
#  define PDEBUG(fmt, ...) \
	fprintf(stderr, "%s(), %s:%d: " fmt,			\
		__func__, __FILE__, __LINE__, ##__VA_ARGS__)
#  define IFDEBUG(x) x
#else
#  define PDEBUG(fmt, ...)
#  define IFDEBUG(x)
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
typedef struct {
	struct list_head list;
	/* DONE: DECLARE NECESSARY MEMBER VARIABLES */
	int index;
	char* addr;
	int order;
} page_t;

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
struct list_head free_area[MAX_ORDER+1];

/* memory area */
char g_memory[1<<MAX_ORDER];

/* page structures */
page_t g_pages[(1<<MAX_ORDER)/PAGE_SIZE];

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/

int get_order_size(int given_size){
	//Find the size to fit the allocating block

	for(int i = MIN_ORDER; i <=MAX_ORDER; i++){
		if( (1 << i) >= given_size){
			return i;
		}
	}
	printf("Given size is bigger than maximum order, invalid\n");
	return -1;
}

/**************************************************************************
 * Local Functions
 **************************************************************************/

/**
 * Initialize the buddy system
 */
void buddy_init()
{
	int i;
	int n_pages = (1<<MAX_ORDER) / PAGE_SIZE;
	for (i = 0; i < n_pages; i++) {
		/* Done: INITIALIZE PAGE STRUCTURES */
		g_pages[i].index = i;
		g_pages[i].addr = PAGE_TO_ADDR(i);
		g_pages[i].order = -1;
	}

	/* initialize freelist */
	for (i = MIN_ORDER; i <= MAX_ORDER; i++) {
		INIT_LIST_HEAD(&free_area[i]);
	}
	/* add the entire memory as a freeblock */
	//Placing a full block as max order
	g_pages[0].order = MAX_ORDER;

	list_add(&g_pages[0].list, &free_area[MAX_ORDER]);
}

/**
 * Allocate a memory block.
 *
 * On a memory request, the allocator returns the head of a free-list of the
 * matching size (i.e., smallest block that satisfies the request). If the
 * free-list of the matching block size is empty, then a larger block size will
 * be selected. The selected (large) block is then splitted into two smaller
 * blocks. Among the two blocks, left block will be used for allocation or be
 * further splitted while the right block will be added to the appropriate
 * free-list.
 *
 * @param size size in bytes
 * @return memory block address
 */
void *buddy_alloc(int size)
{
	/* Done: IMPLEMENT THIS FUNCTION */

	int insert_order = get_order_size(size);

	if(insert_order > 0){

		int check_free = insert_order;

		while(check_free<=MAX_ORDER){

			//Iterate up possible places to insert the block
			if(!(list_empty(&free_area[check_free]))){
				page_t* new_page;

				new_page = list_entry(free_area[check_free].next,page_t,list);
				list_del_init(&(new_page->list));

				//Traverse back to the initial block size and add free areas underneath
				while(insert_order < check_free){

					int buddy_page = ADDR_TO_PAGE(BUDDY_ADDR(new_page->addr,(check_free-1)));

					page_t* new_bud = &g_pages[buddy_page];
					//Change the order to a lower size
					new_bud->order = check_free-1;
					//Add the new buddy to the list below
					list_add(&(new_bud->list),&free_area[check_free-1]);
					//Iterator
					check_free--;
				}

				new_page->order = insert_order;

				void * ret_ptr = (void *)new_page->addr;
				return ret_ptr;

			}else{
				//Check the Next Level for Free Space
				check_free++;
			}

		}


	}else{
		printf("Invalid size to allocate.\n Please Try Again\n");
	}

	return NULL;
}

/**
 * Free an allocated memory block.
 *
 * Whenever a block is freed, the allocator checks its buddy. If the buddy is
 * free as well, then the two buddies are combined to form a bigger block. This
 * process continues until one of the buddies is not free.
 *
 * @param addr memory block address to be freed
 */
void buddy_free(void *addr)
{
	char* address = (char*)addr;
	int index = ADDR_TO_PAGE(address);
	int order = g_pages[index].order;

	while (order <= MAX_ORDER) {
		page_t *buddy_page = NULL;
		// Finds the buddy page of the given address
		struct list_head *temp;
		list_for_each(temp, &free_area[order]) {
			page_t *temp_page = list_entry(temp, page_t, list);
			if(NULL == temp_page ||  BUDDY_ADDR(address, order) == temp_page->addr) {
				buddy_page = temp_page;
			}
		}
		// if buddy page doesn't exist free the given address and add it to the free list
		if(buddy_page == NULL) {
			g_pages[index].order = -1;
			list_add(&g_pages[index].list, &free_area[order]);
			return;
		}
		// reassign the given address if its buddy address is smaller
		if(address > buddy_page->addr) {
				address = buddy_page->addr;
				index = ADDR_TO_PAGE(address);
		}
		// delete the buddy from the list
		list_del(&buddy_page->list);
		// increase the order
		order++;
	}
}
/**
 * Print the buddy system status---order oriented
 *
 * print free pages in each order.
 */
void buddy_dump()
{
	int o;
	for (o = MIN_ORDER; o <= MAX_ORDER; o++) {
		struct list_head *pos;
		int cnt = 0;
		list_for_each(pos, &free_area[o]) {
			cnt++;
		}
		printf("%d:%dK ", cnt, (1<<o)/1024);
	}
	printf("\n");
}
