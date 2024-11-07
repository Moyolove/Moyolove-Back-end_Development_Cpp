#pragma once

#include<mutex>
#include<future>
#include<functional>
#include<memory>
#include<queue>
#include<iostream>
#ifdef WIN32
#include<windows.h>
#else
#include<sys/time.h>
#endif

void getNow(timeval *tv);
int64_t getNowMs();

#define TNOW getNow()
#define TNOWMs getNowMs()

class zero_threadpool{
protected:
    //任务函数
    struct taskFunc
    {
        taskFunc(int64_t expireTime) : _expireTime(expireTime)
        {}
        std::function<void()>  _func;
        int64_t _expireTime = 0;
    };
    typedef std::shared_ptr<taskFunc> taskPtr;

public:
    //构造函数
    zero_threadpool();
    //析构函数，会停止所有线程
    virtual ~zero_threadpool();
    //初始化函数，初始化工作线程个数
    bool init(size_t num);

    //获取工作线程个数，需要上锁，防止读时写
    size_t getThreadNum(){
        std::unique_lock<std::mutex> lock(_mutex);
        return _threads.size();
    }

    //获取当前线程池的任务数
    size_t getJobNum(){
        std::unique_lock<std::mutex> lock(_mutex);
        return _tasks.size();
    }

    //启动所有线程
    bool start();
    //停止所有线程
    void stop();

    //用线程池启用任务，返回future对象
    template<typename F, typename...Args>
    auto exec(F&& f, Args&&...args) -> std::future<decltype(f(args...))> {
        return exec(0, f, args...);
    }

    //带超时控制的线程池启用任务
    template<typename F, typename...Args>
    auto exec(int64_t timeoutMs, F&& f, Args&&...args) -> std::future<decltype(f(args...))> {
        //获取现在时间
        int64_t expireTime = timeoutMs == 0 ? 0 : timeoutMs + TNOWMs;
        //重命名返回值类型
        using retType = decltype(f(args...));
        //封装任务
        auto task = std::make_shared<std::packaged_task<retType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        //封装任务指针，设置过期时间
        taskPtr fPtr = std::make_shared<taskFunc>(expireTime);
        //设置任务执行的函数
        fPtr->_func = [task](){
            (*task)();
        };
        //插入任务，需要上锁
        std::unique_lock<std::mutex> lock(_mutex);
        _tasks.push(fPtr);
        _condition.notify_one();

        return task->get_future();
    }

    //等待所有任务完成，不传参则默认-1，-1表示一直等待
    bool waitForAllDone(int millsecond = -1);
protected:
    //获取任务
    bool get(taskPtr& task);
    //线程池是否退出
    bool isTerminate(){
        return _bTerminate;
    }

    //线程运行态
    void run();

private:
    std::condition_variable _condition;
    std::queue<taskPtr> _tasks;
    std::mutex _mutex;
    std::vector<std::thread*> _threads;
    size_t _threadNum;
    bool _bTerminate;
    std::atomic<int> _atomic{0};
};