//利用C++11 magic static 新特性，自动实现并发阻塞
class Singleton{
public:
    //静态方法只能访问静态成员,即使没有任何类的实例，静态方法仍然能够被调用
    static Singleton& get_Instance(){
        static Singleton _instance;
        return _instance;
    }
private:
    Singleton(){}
    ~Singleton(){}
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

};

