#include<iostream>
#include<setjmp.h>

jmp_buf env;

void func(int arg){
    std::cout << "func" << std::endl;
    longjmp(env, ++arg);
    std::cout << "longjmp complete" << std::endl;
}


int main(){
    int ret = setjmp(env);
    if(ret == 0){
        std::cout << "ret == 0" << std::endl;
        func(ret);
    }else if(ret == 1){
        std::cout << "ret == 1" << std::endl;
        func(ret);
    }

    std::cout << "ret == " <<  ret << std::endl;
}