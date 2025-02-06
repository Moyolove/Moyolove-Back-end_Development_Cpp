

#define MP_ALIGNMENT		32

#define  MP_PAGE_SIZE		4096

#define mp_align(n, a)  (n + (a - 1)) & (~(a-1))

// 17 + 

// 100
//  11

// 17, 4 = 20
// 23, 4 = 24


typedef struct mp_large_s {

	struct mp_large_s *next;
	void *alloc;

} mp_large_t;

typedef struct mp_node_s {

	unsigned char *last;
	unsigned char *end;

	struct mp_node_s *next;
	
	
} mp_node_t;


// 4k
// short
typedef struct mp_pool_s {

	size_t max;

	struct mp_large_s *large;
	struct mp_node_s *current;
	
	mp_node_s head[0]; 
} mp_pool_t;

// sizeof(mp_pool_t)

// malloc (4k)
mp_pool_t * mp_create(size_t size) {  // 4k 

	mp_pool_t *p;
	int ret = posix_memalign(&p, 32, size + sizeof(mp_pool_t) + sizeof(mp_node_t));
	if (ret) {
		return NULL;
	}

	p->max = size;
	p->current = p->head;
	p->large = NULL;

	p->head->last = (unsigned char*)p + sizeof(mp_pool_t) + sizeof(struct mp_node_s);
	p->head->end = p->head->last + size;
		
	return p;

} 

mp_pool_t * mp_destroy(size_t size) {

	

}


void *mp_alloc_large(mp_pool_t *pool, size_t size) {

	void *p = NULL;

	int ret = posix_memalign(&p, MP_ALIGNMENT, size);
	if (ret) return NULL;

	mp_large_t *large = mp_alloc_small(pool, sizeof(mp_large_t));

	large->alloc = p;
	large->next = pool->large;
	pool->large = large;

	return p;
}


void *mp_alloc_small(mp_pool_t *pool, size_t size) {


	unsigned char *m;

	int ret = posix_memalign(&m, MP_ALIGNMENT, pool->max + sizeof(mp_node_t));
	if (ret) return NULL;

	mp_node_t *new_node;

	new_node = (mp_node_t*)m;
	new_node->next = NULL;
	new_node->last = m + sizeof(mp_node_t);
	new_node->end = m + pool->max + sizeof(mp_node_t);

#if 0
	mp_node_t *p = pool->current;
	while (p->next) p = p->next;
	p->next = new_node;
#else

	mp_node_t *p = pool->current;

	new_node->next = p->next;
	pool->current = new_node;

#endif

	m = new_node->last;
	new_node->last += size;

	return m;
	
}



//hook
void *mp_malloc(mp_pool_t *pool, size_t size) {

	if (size > MP_PAGE_SIZE) {
		return mp_alloc_large(pool, size);
	}

	mp_node_t *p = pool->current;

	while (p) {
		
		if (p->end - p->last < size) {
			p = p->next;
			continue;
		}
		unsigned char *m = mp_align(p->last, MP_ALIGNMENT);
		p->last += size; //
		
		return m;
	}

	return mp_alloc_small(pool, size);

}


void mp_free(void *ptr) {

	//for ()

} 


// malloc, calloc
// free

int main() {

	void *ptr = mp_malloc(1024);

}


