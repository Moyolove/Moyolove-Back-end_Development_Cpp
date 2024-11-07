#include<iostream>
#include<thread>
#include<atomic>
#include<vector>
#include<mutex>
#include<condition_variable>
 
std::atomic<bool> ready(false);
std::atomic<bool> winner(false); 

void count_Game(int id){
    while(!ready){ std::cout << "thread" << id << "waiting" << std::endl;}
    std::cout << "thread" << id << "start" << std::endl;
    for(int i = 0; i < 10000000; i++){}
    //第一个走到这的线程，winner此时还为false，判断条件为真，然后将winner置为true，其他线程判断时不会再打印
    if(!winner.exchange(true)) std::cout << "thread" << id << "win" << std::endl;
}

int main(){
    std::vector<std::thread> threads;
    for(int i = 1; i <= 10; i++){
        threads.push_back(std::thread(count_Game, i));
    }
    ready.store(true, std::memory_order_acquire);
    for(auto& t : threads){
        t.join();
    }
    return 0;
}