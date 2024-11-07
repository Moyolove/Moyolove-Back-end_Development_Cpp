#include<iostream>
#include<mutex>
#include<condition_variable>
#include<thread>
#include<deque>
#include<unistd.h>
#include<pthread.h>
#include<chrono>

std::deque<int> q;
std::mutex _mutex;
std::condition_variable cond;
int count = 0;

void func_1(){
    using namespace std::literals::chrono_literals;
    while(true){
        {
            std::unique_lock<std::mutex> lck(_mutex);
            std::cout << "func_1 waiting" << std::endl;
            count++;
            q.push_front(count);
            //notify一般和释放锁要同时，或者说nodify之后要保证自己的锁已经释放
            cond.notify_one();
        }
            std::cout<< "func_1 over" << std::endl;
            std::this_thread::sleep_for(3s);
    }
}

void func_2(){
    using namespace std::chrono_literals;
    while(true){
        {
            std::unique_lock<std::mutex> lck(_mutex);
            std::cout << "func_2 waiting" << std::endl;
            cond.wait(lck, [&](){return !q.empty();});
            auto data = q.back();
            q.pop_back();
            std::cout << "func_2 get from func_1" << data << "\n" << "func_2 over" << std::endl;
        }
    }
}

int main(){
    std::thread t_1(func_1);
    std::thread t_2(func_2);
    t_1.join();
    t_2.join();
    return 0;
}