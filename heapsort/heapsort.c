#include <stdio.h>
#include <unistd.h>

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

static inline void swap_val(struct heao_tree_array *heap, uint16_t p, uint16_t c)
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

static inline uint16_t get_idx_min_child(struct heao_tree_array *heap, uint16_t idx)
{
	return (compare(left(heap, idx), right(heap, min)))
			? right_idx(idx) : left_idx(idx);
}

static inline uint16_t fix_skew(struct heao_tree_array *heap, uint16_t idx)
{
	int rc = 0;
	uint16_t min = get_idx_min_child(heap, idx);

	 //greater than smallest child//min heap
	if (compare(self(heap, idx), self(heap, min))) {
		swap_val(heap, idx, min);
		return min;
	}
	return 0;
}

static inline void sift_up(struct heao_tree_array *heap, uint16_t idx)
{
	if (idx == 1)
		return;

	if (fix_skew(heap, parent_idx(idx))
		sift_up(heap, parent_idx(idx));
}

static inline void sift_down(struct heao_tree_array *heap, uint16_t idx)
{
	uint16_t idx;
	if ((idx = fix_skew(heap, idx)))
		sift_down(heap, idx);
}

static inline int push_elem(struct heap_tree_array *heap, elem_t elem)
{
	if (heap->depth == MAX_ELEM)
		return -1;
	//sift up
	heap->tree[heap->depth++] = elem;
	sift_up(heap, heap->depth);
	return;
}

static inline elem_t pop_elem(struct heap_tree_array *heap)
{
	elem_t rc = EXTREMUM;

	if (heap->depth--) {
		rc = heap->tree[0];
		heap->tree[0] = EXTREMUM;
		sift_down(heap, 0);
	}

	return rc;
}

//Array based max/min-heap
int main(void)
{
	return 0;
}
