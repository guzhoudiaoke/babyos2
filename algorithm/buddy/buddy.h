/*
 * guzhoudiaoke@126.com
 * 2017-11-9
 */

#ifndef _BUDDY_H_
#define _BUDDY_H_

#define MAX_ORDER	(10)
#define MAX_MEM		(128*1024*1024)

#define PAGE_SHIFT	(12)
#define PAGE_SIZE	(1 << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE-1))

/* to align the pointer to the (next) page boundary */
#define PAGE_ALIGN(addr)	(((addr)+PAGE_SIZE-1)&PAGE_MASK)
#define MAP_NR(addr)		(((unsigned long)(addr)) >> PAGE_SHIFT)

typedef struct page_s {
	struct page_s*	next;
	struct page_s*	prev;
} page_t;

typedef struct free_list_s {
	struct free_list_s*	next;
	struct free_list_s*	prev;
	unsigned*			map;
} free_list_t;

typedef struct free_area_s {
	free_list_t free_list[MAX_ORDER+1];
	unsigned char*		base;
} free_area_t;

#endif
