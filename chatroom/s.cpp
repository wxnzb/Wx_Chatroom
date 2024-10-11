#include <iostream>
#include <thread>
#include <arpa/inet.h>
#include <mysql/mysql.h>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "chat.h"
#include "sq.h"
#include "json.h"
#include "ThreadPool.h"
#include <random>
#define HOST "127.0.0.1"
#define PORT 3306
#define USER "root"
#define PASSWD "WwL200488"
#define DBNAME "qq"
#define SERVER_PORT 8888
const int MAX_CLIENTS = 1024;
int yepollfd;

// 新加的
// 设置线程睡眠时间
std::random_device rd; // 真实随机数产生器

std::mt19937 mt(rd()); // 生成计算随机数mt;

std::uniform_int_distribution<int> dist(-1000, 1000); // 生成-1000到1000之间的离散均匀分部数

auto rnd = std::bind(dist, mt);
void simulate_hard_computation()
{
    std::this_thread::sleep_for(std::chrono::milliseconds(2000 + rnd()));
}

void func(int sockfd);

class ChatServer
{
public:
    friend class ThreadPool;
    ChatServer();
    ~ChatServer();
    void run();
    MYSQL *mysql;
    void ynLive(int cfd);

private:
    int serverSockfd;
    pthread_t tid;
    int epollfd;
    ThreadPool pool;
    void addepollcfd(); // 将客户端的cfd加入epoll
    void messageCfd(int cfd);
};
// pool(new ThreadPool(5))
ChatServer::ChatServer() : serverSockfd(socket(AF_INET, SOCK_STREAM, 0)), epollfd(epoll_create(MAX_CLIENTS)), pool(10)
{
    mysql = mysql_init(nullptr);
    if (mysql == nullptr)
    {
        std::cerr << "mysql_init() failed\n";
        return; // 退出或进行其他错误处理
    }

    mysql_real_connect(mysql, HOST, USER, PASSWD, DBNAME, PORT, nullptr, 0);

    yepollfd = epollfd;
    if (serverSockfd < 0)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }
    // 设置端口复用
    int opt = 1;
    if (setsockopt(serverSockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("Failed to set socket options");
        close(serverSockfd);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("0.0.0.0");
    addr.sin_port = htons(SERVER_PORT);
    if (bind(serverSockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Bind failed");
        close(serverSockfd);
        exit(EXIT_FAILURE);
    }

    if (listen(serverSockfd, 100) < 0)
    {
        perror("Listen failed");
        close(serverSockfd);
        exit(EXIT_FAILURE);
    }
    struct epoll_event event = {};
    event.events = EPOLLIN | EPOLLET; //
    event.data.fd = serverSockfd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, serverSockfd, &event);
    std::cout << "epollfd" << epollfd << std::endl;
}

ChatServer::~ChatServer()
{
    close(serverSockfd);
    close(epollfd);
    pool.shutdown(); // 关闭线程池
    mysql_close(mysql);
}
void ChatServer::addepollcfd() // 将客户端的cfd加入epoll
{
    int cfd = accept(serverSockfd, nullptr, nullptr);
    // fcntl(cfd, F_SETFL, fcntl(cfd, F_GETFD, 0) | O_NONBLOCK); // 将他设置成非阻塞,这有大问题

    struct epoll_event event = {};
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = cfd;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, cfd, &event) == -1)
    {
        perror("Epoll_ctl accept failed");
        close(cfd);
    }
}
void ChatServer::messageCfd(int cfd) // 已有连接传来消息
{
    // 从epoll中删除当前文件描述符
    struct epoll_event event;
    event.events = EPOLLIN | EPOLLET;
    event.data.fd = cfd;

    if (epoll_ctl(yepollfd, EPOLL_CTL_DEL, cfd, &event) == -1)
    {
        perror("Epoll_ctl-DEL failed");
        exit(EXIT_FAILURE);
    }
    pool.submit(func, cfd);

    return;
}
void ChatServer::ynLive(int cfd)
{
    int keep_alive = 1;     // 开启TCP心跳机制
    int keep_idle = 10000;  // 开始首次心跳监测的TCP空闲时间
    int keep_interval = 10; // 两次心跳检测的时间
    int keep_count = 5;     // 连接心跳检测失败次数后，判定连接失效

    // 开启TCP心跳机制
    if (setsockopt(cfd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keep_alive, sizeof(keep_alive)) == -1)
    {
        perror("setsockopt SO_KEEPALIVE");
        return;
    }

    // 设置首次心跳检测前的TCP空闭时间
    if (setsockopt(cfd, IPPROTO_TCP, TCP_KEEPIDLE, (void *)&keep_idle, sizeof(keep_idle)) == -1)
    {
        perror("setsockopt TCP_KEEPIDLE");
        return;
    }

    // 设置两次心跳检测的间隔时间
    if (setsockopt(cfd, IPPROTO_TCP, TCP_KEEPINTVL, (void *)&keep_interval, sizeof(keep_interval)) == -1)
    {
        perror("setsockopt TCP_KEEPINTVL");
        return;
    }

    // 设置连续心跳检测失败次数后，判定连接失效
    if (setsockopt(cfd, IPPROTO_TCP, TCP_KEEPCNT, (void *)&keep_count, sizeof(keep_count)) == -1)
    {
        perror("setsockopt TCP_KEEPCNT");
        return;
    }

    std::cout << "TCP Keepalive 已配置" << std::endl;
}
void ChatServer::run()
{
    // pool->init();
    struct epoll_event events[MAX_CLIENTS];
    while (true)
    {
        int n = epoll_wait(epollfd, events, MAX_CLIENTS, -1);
        for (int i = 0; i < n; i++)
        {
            if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP) || (!(events[i].events & EPOLLIN)))
            {
                printf("%d\n",(events[i].events));
            //  std::cout << "哈哈哈哈" << std::endl;
                fprintf(stderr, "epoll error\n");
                //新加的，将异常突出的给他改成离线
                struct protocol msg;
                Person p1(mysql, msg, events[i].data.fd);
                p1.offline();
                close(events[i].data.fd);
                continue;
            }

            if (events[i].data.fd == serverSockfd)
            {
                addepollcfd(); // 新的连接
                std::cout << "new connection" << std::endl;
            }
            else if (events[i].events & EPOLLIN)
            { // 已有连接传来消息
                // 是否应该县监测是否在线
                std::cout << "已经连接的epollfd说话了" << events[i].data.fd;
                ynLive(events[i].data.fd);
                messageCfd(events[i].data.fd);
            }
        }
    }
}
void func(int sockfd) // 新学的知识
{
    // MYSQL *mysql=sever.mysql;
    // 新加的
    // simulate_hard_computation();
    std::cout << "new thread" << std::endl;
    MYSQL *mysql = mysql_init(nullptr);
    if (mysql == nullptr)
    {
        std::cerr << "mysql_init() failed\n";
        return; // 退出或进行其他错误处理
    }

    mysql_real_connect(mysql, HOST, USER, PASSWD, DBNAME, PORT, nullptr, 0);

    //    while (1)
    //    {
    std::cout << "yepollfd" << yepollfd << std::endl;
    protocol msg = receive_data(sockfd);
    std::cout << "到底收到了没" << std::endl;
    std::cout << "cmd: " << msg.cmd << std::endl;
    // 现试一下麻烦的办法，现在没想出好的办法11
    if (msg.state == OFFLINE)
    { // 客户端断开连接
        Person p1(mysql, msg, sockfd);
        p1.offline();
        return;
    }
    Person person(mysql, msg, sockfd);
    std::cout << "Command: " << msg.cmd << std::endl;
    switch (msg.cmd)
    {
    case REGISTE:
        person.registerUser(); // 注册
        break;
    case LOGIN:
        person.loginUser(); // 登录
        break;
    case LOGOFF:
        person.logoffUser(); // 下线
        break;
    case ADDFRIEND:
        std::cout << "add friend" << std::endl;
        person.addFriend(); // 添加好友//自己的cfd,朋友的msg
        break;
    case BLOCKFRIEND:
        person.blockFriend(); // 屏蔽好友
        break;
    case DELETEFRIEND:
        person.deleteFriend(); // 删除好友
        break;
    case STATUSFRIEND:
        std::cout << "status friend" << std::endl;
        person.statusFriend(); // 查看好友状态
        break;
    case YNACCEPT:
        person.ynAccept(); // 同意/拒绝添加好友
        break;
    case PRIVATECHAT:
        std::cout << "private chat" << std::endl;
        person.privateChat(); // 私聊
        break;
    case FRIENDNOTICE:
        person.friendNotice(); // 好友通知
        break;
    case UNBLOCKFRIEND:
        person.unblockFriend(); // 取消屏蔽好友
        break;
    case CHATFRIENDRECORD:
        person.chatfriendRecord(); // 查看好友聊天记录
        break;
    case CREATEGROUP:
        std::cout << "create group" << std::endl;
        person.createGroup(); // 创建群聊
        break;
    case DELETEGROUP:
        person.deleteGroup(); // 删除群聊
        break;
    case REMOVEUSER:
        person.removeUser(); // 移除群聊成员
        break;
    case GROUPLIST:
        person.groupList(); // 查看群聊列表
        break;
    case IINGROUP:
        person.iinGroup(); // 查看自己所在的群
        break;
    case ADMANAGER:
        person.adManager(); // 添加删除管理员
        break;
    case APPLYADDGROUP:
        person.applyaddGroup(); // 申请加入群聊
        break;
    case YNACCEPTGROUP:
        person.ynacceptGroup(); // 同意拒绝申请
        break;
    case GROUPNOTICE:
        person.groupNotice(); // 群聊通知
        break;
    case GROUPCHAT:
        person.groupChat(); // 群聊
        break;
    case GROUPCHATRECORD:
        person.groupchatRecord(); // 查看群聊记录
        break;
    case CHECKFILE:
        person.checkFile(); // 检查接收文件的人或群
        break;
    case SENDFILE:
        person.sendFile(); // 发送文件
        break;
    case RECEIVEFILE:
        person.receiveFile(); // 接收文件
        break;
    case RECEIVEOK:
        person.receiveOk(); // 文件接收成功
        break;
    } // default:
      //     std::cerr << "Unknown command." << std::endl;
      //     break;
      //  }
      // 在把他加上
    struct epoll_event event1 = {};
    event1.events = EPOLLIN;
    event1.data.fd = sockfd;

    if (epoll_ctl(yepollfd, EPOLL_CTL_ADD, sockfd, &event1) == -1)
    {
        std::cout << "你好像眉山除掉" << std::endl;
        perror("Epoll_ctl ADD failed");
    }
    else
    {
        std::cout << "添加成功" << std::endl;
    }

    mysql_close(mysql);
    // close(sockfd);
    return;
}
int main()
{
    ChatServer server;
    server.run();
    return 0;
}