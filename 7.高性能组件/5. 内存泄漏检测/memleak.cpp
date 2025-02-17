#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<link.h>

#define _GNU_SOURCE
#include<dlfcn.h>

#if 0
//int count = 0;

void* _malloc(size_t size, const char* filename, int linenum){
    //count++;
    void *ptr = malloc(size);

    char file[128] = {0};
    sprintf(file, "./mem/%p.mem", ptr);

    FILE *fp = fopen(file, "w");
    fprintf(fp, "[+], filename: %s, linenum: %d\n", filename, linenum);

    fflush(fp);
    fclose(fp);

    //printf("malloc, filename: %s, linenum: %d\n", filename, linenum);
    return ptr;
}

void _free(void *ptr, const char* filename, int linenum){
    //count--;
    char file[128] = {0};
    sprintf(file, "./mem/%p.mem", ptr);

    if(unlink(file) < 0){ //文件不存在
        printf("too much free\n");
        return;
    }

    //printf("free, filename: %s, linenum: %d\n", filename, linenum);
    free(ptr);
}


#define malloc(size) _malloc(size, __FILE__, __LINE__)
#define free(ptr) _free(ptr, __FILE__, __LINE__)

#elif 1
//hook
//definition
typedef void* (*malloc_t)(size_t size );
malloc_t malloc_f = NULL; 

typedef void (*free_t)(void* ptr);
free_t free_f = NULL;

int enable_malloc_hook = 1;
int enable_free_hook = 1;
//implement
void* malloc(size_t size){
    void* ptr = NULL;
    if(enable_malloc_hook){
        enable_malloc_hook = 0;
        printf("malloc \n");
        ptr = malloc_f(size);

        void* caller = __builtin_return_address(0);
        char file[128] = {0};
        sprintf(file, "./mem/%p.mem", ptr);

        FILE *fp = fopen(file, "w");
        fprintf(fp, "[+], caller: %p, addr: %p, size: %ld", caller, ptr, size);

        fflush(fp);

        enable_malloc_hook = 1;
    }else{
        ptr = malloc_f(size);
    }
    return ptr;
}

void free(void* ptr){
    if(enable_free_hook){
        enable_free_hook = 0;
        char file[128] = {0};
        sprintf(file, "./mem/%p.mem", ptr);

        if(unlink(file) < 0){ //文件不存在
            printf("too much free\n");
            return;
        }

        //printf("free, filename: %s, linenum: %d\n", filename, linenum);
        free_f(ptr);
        enable_free_hook = 1;
    }else{
        free_f(ptr);
    }
    return;
}

//init
void init_hook(){
    if(!malloc_f){
        malloc_f = (malloc_t)dlsym(RTLD_NEXT, "malloc");
    }
    if(!free_f){
        free_f = (free_t)dlsym(RTLD_NEXT, "free");
    }
}

#elif 0

extern void* __libc_malloc(size_t size);
extern void __libc_free(void* ptr);

int enable_malloc_hook = 1;
int enable_free_hook = 1;
//implement
void* malloc(size_t size){
    void* ptr = NULL;
    if(enable_malloc_hook){
        enable_malloc_hook = 0;
        printf("malloc \n");
        ptr = __libc_malloc(size);

        void* caller = __builtin_return_address(0);
        char file[128] = {0};
        sprintf(file, "./mem/%p.mem", ptr);

        FILE *fp = fopen(file, "w");
        fprintf(fp, "[+], caller: %p, addr: %p, size: %ld", caller, ptr, size);

        fflush(fp);

        enable_malloc_hook = 1;
    }else{
        ptr = __libc_malloc(size);
    }
    return ptr;
}

void free(void* ptr){
    if(enable_free_hook){
        enable_free_hook = 0;
        char file[128] = {0};
        sprintf(file, "./mem/%p.mem", ptr);

        if(unlink(file) < 0){ //文件不存在
            printf("too much free\n");
            return;
        }

        //printf("free, filename: %s, linenum: %d\n", filename, linenum);
        __libc_free(ptr);
        enable_free_hook = 1;
    }else{
        __libc_free(ptr);
    }
    return;
}


#endif


#if 1

int main(){
    init_hook();
    void *p1 = malloc(5);
    void *p2 = malloc(15);
    void *p3 = malloc(25);

    free(p1);
    free(p3);

    //printf("count = %d\n", count);
    return 0;
}

#endif