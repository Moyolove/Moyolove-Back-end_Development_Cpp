//利用模板和继承实现变化点
template<typename T>
class Singleton{
public:
    //静态方法只能访问静态成员,即使没有任何类的实例，静态方法仍然能够被调用
    static T& get_Instance(){
        static T _instance;
        return _instance;
    }
protected://使得子类能够继承而其他类访问不到
    Singleton(){}
    virtual ~Singleton(){}
private:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;
};

class Designpattern : public Singleton<Designpattern>{
    //使得父类能够调用子类的构造函数
    friend class Singleton<Designpattern>;
private:
    Designpattern(){}
    ~Designpattern(){}
};