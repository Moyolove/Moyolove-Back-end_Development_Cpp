

#include <stdio.h>

#include "ngx_config.h"
#include "ngx_conf_file.h"
#include "nginx.h"
#include "ngx_core.h"
#include "ngx_string.h"
#include "ngx_palloc.h"
#include "ngx_array.h"
#include "ngx_hash.h"


volatile ngx_cycle_t  * ngx_cycle;


#define unused(x)	(x)=(x)

void
ngx_log_error_core(ngx_uint_t level, ngx_log_t *log, ngx_err_t err,
    const char *fmt, ...) {

	unused(level);
	unused(log);
	unused(err);
	unused(fmt);

	
}




typedef struct  {

	int name;
	int github;

} ngx_teacher_t;


void print_pool(ngx_pool_t *pool) {

	while (pool) {

		printf("avail pool memory size: %ld\n\n\n", 
				pool->d.end - pool->d.last);

		pool = pool->d.next;

	}

}


// string
// list
// queue
// array
// hash
// palloc
// 
int main() {

#if 0
	ngx_str_t name = ngx_string("king");

	printf("name len: %ld\n", name.len);
	printf("name data: %s\n", name.data);

#elif 0

	ngx_pool_t *pool;

	pool = ngx_create_pool(1024, NULL);

	print_pool(pool);

	ngx_array_t *arr = ngx_array_create(pool, 32, sizeof(ngx_teacher_t));

	print_pool(pool);

	ngx_teacher_t *t1 = ngx_array_push(arr);
	t1->name = 13;
	t1->github = 15;
	//t1->name = ngx_string("king"); //
	//t1->github = ngx_string("github.com/wangbojing");

	print_pool(pool);
	
	ngx_teacher_t *t2 = ngx_array_push(arr);
	t2->name = 20;
	t2->github = 25;
	//t2->name = ngx_string("xxxx");
	//t2->github = ngx_string("github.com/xxxx");

	print_pool(pool);

#else

	ngx_pool_t *pool;

	pool = ngx_create_pool(1024, NULL);

	print_pool(pool);

	ngx_list_t *list = ngx_list_create(pool, 32, sizeof(ngx_teacher_t));

	print_pool(pool);

	ngx_teacher_t *t1 = ngx_list_push(list);
	t1->name = 13;
	t1->github = 15;

	print_pool(pool);

	ngx_teacher_t *t2 = ngx_list_push(list);
	t2->name = 20;
	t2->github = 25;

	print_pool(pool);

#endif

}


