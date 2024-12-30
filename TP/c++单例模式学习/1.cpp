#include<iostream>
class NOdl{
    public:
        NOdl():age(0){}
        void setAge(int a)
        {
            age=a;
        }
        int getAge()
        {
            return age;
        }
        private:
        int age;
};
int main()
{
    NOdl a;
    NOdl b;
    a.setAge(10);
    b.setAge(20);
    std::cout<<a.getAge()<<std::endl;
    std::cout<<b.getAge()<<std::endl;
    return 0;
}