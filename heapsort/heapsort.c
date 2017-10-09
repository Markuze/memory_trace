#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <stdbool.h>
#include <stdlib.h>

#define EXTREMUM UINT_MAX

#define compare(a,b) g((a),(b)) //is a greater than b
#define MAX_ELEM 28
#define elem_t uint32_t

struct heap_tree_array {
	uint16_t depth;
	elem_t tree[MAX_ELEM];
};

static inline bool g(elem_t a, elem_t b)
{
	return (a > b);
}

static inline void swap_val(struct heap_tree_array *heap, uint16_t p, uint16_t c)
{
	heap->tree[p -1] ^= heap->tree[c -1];
	heap->tree[c -1] ^= heap->tree[p -1];
	heap->tree[p -1] ^= heap->tree[c -1];
}


//These are logical idxes. logicaly the first idx is 1. and not the natural zero.
static inline uint16_t parent_idx(uint16_t idx) {return idx >> 1;}
static inline uint16_t left_idx(uint16_t idx) {return idx << 1;}
static inline uint16_t right_idx(uint16_t idx) {return (idx << 1) + 1;}

static inline elem_t parent(struct heap_tree_array *heap, uint16_t idx)
{
	if (heap->depth < idx)
		return UINT_MAX;// should panic
	return heap->tree[parent_idx(idx) - 1];
}

static inline elem_t self(struct heap_tree_array *heap, uint16_t idx)
{
	if (heap->depth < idx)
		return UINT_MAX;// should panic
	return heap->tree[idx - 1];
}

static inline elem_t left(struct heap_tree_array *heap, uint16_t idx)
{
	if (heap->depth < left_idx(idx))
		return EXTREMUM;
	return heap->tree[left_idx(idx) - 1];
}

static inline elem_t right(struct heap_tree_array *heap, uint16_t idx)
{
	if (heap->depth < right_idx(idx))
		return EXTREMUM;
	return heap->tree[right_idx(idx) - 1];
}

static inline uint16_t get_idx_min_child(struct heap_tree_array *heap, uint16_t idx)
{
	return (compare(left(heap, idx), right(heap, idx)))
			? right_idx(idx) : left_idx(idx);
}

static inline uint16_t fix_skew(struct heap_tree_array *heap, uint16_t idx)
{
	int rc = 0;
	uint16_t min = get_idx_min_child(heap, idx);

	//printf("parent %d min idx %d: %u >? %u", idx, min, self(heap, idx), self(heap, min));
	 //greater than smallest child//min heap
	if (compare(self(heap, idx), self(heap, min))) {
		swap_val(heap, idx, min);
		//printf("...yes\n");
		return min;
	}
	//printf("...no\n");
	return 0;
}

static inline void sift_up(struct heap_tree_array *heap, uint16_t idx)
{
	if (idx == 1)
		return;
//printf("%s:<%d> [%d](%u) :[%d](%u):[%d](%u)\n", __FUNCTION__,
//	heap->depth,
//	idx, self(heap, idx),
//	left_idx(idx), left(heap, idx),
//	right_idx(idx), right(heap, idx));
	if (fix_skew(heap, parent_idx(idx)))
		sift_up(heap, parent_idx(idx));
}

static inline void sift_down(struct heap_tree_array *heap, uint16_t idx)
{
	uint16_t child;
	if ((child = fix_skew(heap, idx)))
		sift_down(heap, child);
}

int push_elem(struct heap_tree_array *heap, elem_t elem)
{
	if (heap->depth == MAX_ELEM)
		return -1;
	//sift up
	heap->tree[heap->depth++] = elem;
	sift_up(heap, heap->depth);
	return 0;
}

elem_t pop_elem(struct heap_tree_array *heap)
{
	elem_t rc = EXTREMUM;

	if (heap->depth--) {
		rc = heap->tree[0];
		heap->tree[0] = EXTREMUM;
		sift_down(heap, 1);
	}

	return rc;
}

//Array based max/min-heap
#define MAX MAX_ELEM
struct heap_tree_array heap;
static inline void dump(void)
{
	int i;
	for (i = 0; i < MAX; i++) {
		printf("%4d", i + 1);
	}
	printf("\n");
	for (i = 0; i < MAX; i++) {
		printf("%4u",heap.tree[i]);
	}
	printf("\n");

}
int main(void)
{
	int i;
	int min = UINT_MAX;
	elem_t elem;
	srand(time(NULL));

	for (i = 0; i < MAX; i++) {
		elem_t r = rand() % 256;
		min = (r < min) ? r : min;
	//	printf("pushing %u\n", r);
		push_elem(&heap, r);
	//	printf("top %u\n", heap.tree[0]);
		if (min != heap.tree[0]) {
			printf("ERROR: min is %u\n", min);
			return(-1);
		}

	}
	dump();
	while ((elem = pop_elem(&heap)) != EXTREMUM) {
		printf("%u ", elem);
	}
	printf("...\n");
	return 0;
}
