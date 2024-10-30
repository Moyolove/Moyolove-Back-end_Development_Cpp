#include<cstdlib>
#include<mutex>
//加锁，但未解决C++11 CPU指令重排的问题，可以使用原子操作和内存栅栏解决
class Singleton{
public:
    //静态方法只能访问静态成员,即使没有任何类的实例，静态方法仍然能够被调用
    static Singleton* get_Instance(){
        //double check
        if(_instance == nullptr){
            std::lock_guard<std::mutex> lock(_mutex);
            if(_instance == nullptr){
                _instance = new Singleton();
                atexit(destructor);
            }
        }
        return _instance;
    }
private:
    static void destructor(){
        if(_instance != nullptr){
            delete _instance;
            _instance = nullptr;
        }
    }

    Singleton(){}
    ~Singleton(){}
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    static Singleton* _instance;
    static std::mutex _mutex;
};

Singleton* Singleton::_instance = nullptr;
std::mutex Singleton::_mutex;