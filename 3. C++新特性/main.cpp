#include<iostream>
#include<string>
#include"zero_threadpool.h"

void func_0(){
    std::cout << "func_0()" << std::endl;
}

void func_1(int a){
    std::cout << "func_1 a=" << a << std::endl;
}

void func_2(std::string a, std::string b){
    std::cout << "func_2() a=" << a << " b=" << b << std::endl;
}

void test_01(){
    zero_threadpool threadpool;
    threadpool.init(2);
    threadpool.start();
    threadpool.exec(1000, func_0);
    threadpool.exec(func_1, 10);
    threadpool.exec(func_2, "Moyo", "Sun");
    threadpool.waitForAllDone();
    threadpool.stop();
}

int func_1_future(int a){
    std::cout << "func_1_future a=" << a << std::endl;
    return a;
}

std::string func_2_future(int a, std::string b){
    std::cout << "func_2_future a=" << a << std::endl;
    return b;
}

void test_02(){
    zero_threadpool threadpool;
    threadpool.init(2);
    threadpool.start();
    auto result_1 = threadpool.exec(func_1_future, 10);
    auto result_2 = threadpool.exec(func_2_future, 20, "Moyo");
    std::cout << "result_1="  << result_1.get() << std::endl;
    std::cout << "result_2="  << result_2.get() << std::endl;
    threadpool.waitForAllDone();
    threadpool.stop();
}

int main(){
    test_01();
    test_02();
    return 0;
}