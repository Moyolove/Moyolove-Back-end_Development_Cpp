#include<iostream>

class Prostrategy{
public:
    virtual void strategy() = 0;
    virtual ~Prostrategy(){};
};

class Spring_Festival : public Prostrategy{
public:
    virtual void strategy(){
        std::cout << "Spring_Festival, -%50" << std::endl;
    }
    ~Spring_Festival(){}
};

class Midautumn_Festival : public Prostrategy{
public:
    virtual void strategy(){
        std::cout << "Midautumn_Festival, -30%" << std::endl;
    }
    ~Midautumn_Festival(){}
};

class Promotion{
public:
    Promotion(Prostrategy *sss) : s(sss){}
    ~Promotion(){}
    void Do_promotion(){
        s->strategy();
    }

private:
    Prostrategy *s;
};

int main(){
    Prostrategy *s = new Spring_Festival();
    Promotion *p = new Promotion(s);
    p->Do_promotion();
    delete s;
    delete p;
    return 0;
}