#include<iostream>
class YESdl{
    public:
    static YESdl& GetOnedl(){
        static YESdl ONEdl;
        return ONEdl;
    }
    void Setage(int a){
        age=a;
    }
    int Getage() const{
        return age;
    }
    YESdl(const YESdl&)=delete;
    YESdl& operator=(const YESdl&)=delete;
    private:
    YESdl():age(0){}
    int age;
};
int main()
{
    YESdl& a= YESdl::GetOnedl();
    a.Setage(10);
    std::cout<<a.Getage()<<std::endl;
    YESdl& b= YESdl::GetOnedl();
    std::cout<<b.Getage()<<std::endl;
    a.Setage(100);
    std::cout<<a.Getage()<<std::endl;
     std::cout<<b.Getage()<<std::endl;

}