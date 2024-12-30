#include<iostream>
#include<vector>
#include<thread>
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
   // ~YESdl(){}不需要，静态局部变量：static YESdl ONEdl; 确保了 ONEdl 只在第一次调用 GetOnedl 时创建，并在程序结束时自动销毁。
    int age;
};
// int main()
// {
//     YESdl& a= YESdl::GetOnedl();
//     a.Setage(10);
//     std::cout<<a.Getage()<<std::endl;
//     YESdl& b= YESdl::GetOnedl();
//     std::cout<<b.Getage()<<std::endl;
//     a.Setage(100);
//     std::cout<<a.Getage()<<std::endl;
//     std::cout<<b.Getage()<<std::endl;
// }
//这个虽然是懒汉是，但是C++11下是线程安全的
void threadfunc(int id)
{
    YESdl& a=YESdl::GetOnedl();
    a.Setage(id);
    std::cout<<a.Getage()<<std::endl;
}
int main()
{
    const int num_thread=32000;
    std::vector<std::thread>threads;
    for(int i=0;i<num_thread;i++)
    {
        threads.emplace_back(threadfunc,i);
    }
    for(auto& th:threads)
    {
        th.join();
    }
    std::cout<<YESdl::GetOnedl().Getage()<<std::endl;
    return 0;
}