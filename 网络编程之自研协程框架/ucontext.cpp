//ucontext

//getcontext(&context)

//makecontext(&context, func, arg)

//swapcontext(&current_context, &next_context)

#include<iostream>
#include<ucontext.h>

ucontext_t ctx[2];
ucontext_t main_ctx;
int count = 0;

void func_1(){
    while(count++ < 100){
        std::cout << "1";
        swapcontext(&ctx[0], &ctx[1]);
        std::cout << "3";
    }
}

void func_2(){
    while(count++ < 100){
        std::cout << "2";
        swapcontext(&ctx[1], &ctx[0]);
        std::cout << "4";
    }
}

int main(){
    char stack_1[2048] = {0};
    char stack_2[2048] = {0};

    getcontext(&ctx[0]);
    ctx[0].uc_stack.ss_sp = stack_1;
    ctx[0].uc_stack.ss_size = sizeof(stack_1);
    ctx[0].uc_link = &main_ctx;
    makecontext(&ctx[0], func_1, 0);

    getcontext(&ctx[1]);
    ctx[1].uc_stack.ss_sp = stack_1;
    ctx[1].uc_stack.ss_size = sizeof(stack_1);
    ctx[1].uc_link = &main_ctx;
    makecontext(&ctx[1], func_2, 0);

    std::cout << "swapcontext" << std::endl;

    swapcontext(&main_ctx, &ctx[0]);

    std::cout << "end\n" << std::endl;
}