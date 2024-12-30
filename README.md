>**项目依赖：**
- hiredis
- mysql
- json
>**使用方法：**
>**下载源码：**

git clone git@github.com:wxnzb/learn.git

cd chroom

>**安装依赖：**
>
**ubuntu:**

下载hiredis:sudo apt install libhiredis-dev  

下载mysql：sudo apt install mysql-server

下载json库：sudo apt install nlohmann-json3-dev

**arch:**

下载hiredis:sudo pacman -S hiredis

下载mysql：sudo pacman -S mariadb

下载json库：sudo pacman -S nlohmann-json

**编译运行**

 mkdir build && cd build
	
 cmake ..
	
**只编译客户端**

  make 
		
  ./c
		
**只编译服务端**

  make 
		
  ./s
