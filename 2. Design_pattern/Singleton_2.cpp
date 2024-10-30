#include<cstdlib>
//程序结束时调用atexit主动释放内存,不是线程安全的
class Singleton{
public:
    //静态方法只能访问静态成员,即使没有任何类的实例，静态方法仍然能够被调用
    static Singleton* get_Instance(){
        if(_instance == nullptr){
            _instance = new Singleton();
            atexit(destructor);
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
};

Singleton* Singleton::_instance = nullptr;
