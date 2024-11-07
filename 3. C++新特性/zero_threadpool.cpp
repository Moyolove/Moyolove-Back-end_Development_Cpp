#include"zero_threadpool.h"

zero_threadpool::zero_threadpool() : _threadNum(1), _bTerminate(false)
{}

zero_threadpool::~zero_threadpool(){
    stop();
}

bool zero_threadpool::init(size_t num){
    std::unique_lock<std::mutex> lock(_mutex);
    if(!_threads.empty()){
        return false;
    }
    _threadNum = num;
    return true;
}

bool zero_threadpool::start(){
    std::unique_lock<std::mutex> locker(_mutex);
    if(!_threads.empty()){
        return false;
    }
    for(size_t i = 0; i < _threadNum; i++){
        _threads.push_back(new std::thread(&zero_threadpool::run, this));
    }

    return true;
}

void zero_threadpool::stop(){
    {
        std::unique_lock<std::mutex> locker(_mutex);

        _bTerminate = true;

        _condition.notify_all();
    }

    for(size_t i = 0; i < _threads.size(); i++){
        if(_threads[i]->joinable()){
            _threads[i]->join();
        }
        delete _threads[i];
        _threads[i] = nullptr;
    }
    std::unique_lock<std::mutex> locker(_mutex);
    _threads.clear();
}

bool zero_threadpool::get(taskPtr& task){
    std::unique_lock<std::mutex> locker(_mutex);
    if(_tasks.empty()){
        _condition.wait(locker, [this](){return _bTerminate || !_tasks.empty();});
    }
    if(_bTerminate){
        return false;
    }
    if(!_tasks.empty()){
        task = std::move(_tasks.front());
        _tasks.pop();
        return true;
    }
    return false;
}

void zero_threadpool::run(){
    while(!isTerminate()){
        taskPtr task;
        bool ok = get(task);
        if(ok){
            ++_atomic;
            try
            {
                if(task->_expireTime != 0 && task->_expireTime < TNOWMs){
                    //任务超时，根据业务处理
                }
                else{
                    task->_func();
                }
            }
            catch(...)
            {
            }
            --_atomic;
            std::unique_lock<std::mutex> locker(_mutex);
            //通知waitForAllDone
            if(_atomic == 0 && _tasks.empty()){
                _condition.notify_all();
            }
        }
    }
}

bool zero_threadpool::waitForAllDone(int millseconds){
    std::unique_lock<std::mutex> locker(_mutex);
    if(_tasks.empty()){
        return true;
    }
    if(millseconds < 0){
        _condition.wait(locker, [this](){return _tasks.empty();});
        return true;
    }
    if(millseconds > 0){
        return _condition.wait_for(locker, std::chrono::milliseconds(millseconds), [this](){return _tasks.empty();});
    }
    return false;
}

int getTimeOfDay(struct timeval &tv){
#if WIN32
#else
    return ::gettimeofday(&tv, 0);
#endif
}

void getNow(timeval *tv)
{
#if TARGET_PLATFORM_IOS || TARGET_PLATFORM_LINUX

    int idx = _buf_idx;
    *tv = _t[idx];
    if(fabs(_cpu_cycle - 0) < 0.0001 && _use_tsc)
    {
        addTimeOffset(*tv, idx);
    }
    else
    {
        TC_Common::gettimeofday(*tv);
    }
#else
    gettimeofday(tv, 0);
#endif
}

int64_t getNowMs()
{
    struct timeval tv;
    getNow(&tv);

    return tv.tv_sec * (int64_t)1000 + tv.tv_usec / 1000;
}