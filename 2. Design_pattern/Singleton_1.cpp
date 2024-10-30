//此版本有内存泄漏，因为无法析构导致无法释放堆内存
class Singleton{
public:
    //静态方法只能访问静态成员,即使没有任何类的实例，静态方法仍然能够被调用
    static Singleton* get_Instance(){
        if(_instance == nullptr){
            _instance = new Singleton();
        }
        return _instance;
    }
private:
    Singleton(){}
    ~Singleton(){}
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

    static Singleton* _instance;
};

Singleton* Singleton::_instance = nullptr;

