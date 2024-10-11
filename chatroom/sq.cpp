#include "sq.h"
#include <cstring>
#include "chat.h"
#include "json.h"
// 在类定义之外定义并初始化静态成员变量
std::vector<std::map<int, int>> Person::mang;

Person::Person(MYSQL *Mysql, protocol &Msg, int Sockfd) : mysql(Mysql), msg(Msg), sockfd(Sockfd)
{
}

Person::~Person()
{
}

// 加人
int Person::addUser()
{
    char sql_cmd[256];
    std::cout << "名字" << msg.name << "密码" << msg.data << std::endl; // 打印名字和密码
    std::cout << "客户端的cfd" << sockfd << std::endl;
    // 直接加入数据库，并将状态设置成在线
    snprintf(sql_cmd, sizeof(sql_cmd), "insert into userdata(username, password,cfd,status) values('%s', '%s','%d','%d');", msg.name.c_str(), msg.data.c_str(), sockfd, 1);

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    int insert_id = mysql_insert_id(mysql); // 获取自动分配的id
    std::cout << "得到自增的自动分配的id为" << insert_id << std::endl;

    return insert_id;
}
// 减人
int Person::deleteUser() // 要删掉userdata里面的信息,还有friendlist里面的信息,还有他所加的群也要退掉他，如果是群主，则将这个群解散
{
    std::cout << msg.name << msg.data << std::endl;
    std::cout << msg.name << msg.data << std::endl;
    std::cout << sockfd << std::endl;
    // 要删掉userdata里面的信息,
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "delete from userdata where id='%d';", msg.id);
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    // friendlist里面的信息
    char sql_cmd1[256];
    snprintf(sql_cmd1, sizeof(sql_cmd1), "delete from friendlist where (usera='%d' or userb='%d');", msg.id, msg.id);
    ret = mysql_query(mysql, sql_cmd1);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    // 他所加的群也要退掉他，如果是群主，则将这个群解散
    char sql_cmd2[256];
    snprintf(sql_cmd2, sizeof(sql_cmd2), "select name,type from groupdata where userid='%d';", msg.id);
    ret = mysql_query(mysql, sql_cmd2);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    if (result == NULL)
    {
        std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
        return -1;
    }

    int num_rows = mysql_num_rows(result);
    for (int i = 0; i < num_rows; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row == NULL)
        {
            std::cerr << "[ERR] mysql fetch row error: " << mysql_error(mysql) << std::endl;
            break;
        }
        if (atoi(row[1]) == 1) // 表示他是群主，将这个群一删除
        {
            char sql_cmd3[256];
            snprintf(sql_cmd3, sizeof(sql_cmd3), "delete from groupdata where name='%s';", row[0]);
            ret = mysql_query(mysql, sql_cmd3);
            if (ret != 0)
            {
                std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
                return -1; // Error indicator
            }
        }
        else
        {
            char sql_cmd4[256];
            snprintf(sql_cmd4, sizeof(sql_cmd4), "delete from groupdata where id='%d';", msg.id);
            ret = mysql_query(mysql, sql_cmd4);
            if (ret != 0)
            {
                std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
                return -1; // Error indicator
            }
        }
    }
    return 0;
}
// 根据cfd找id
int Person::findId()
{
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select id from userdata where cfd = '%d';", sockfd);

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row != nullptr)
    {
        return atoi(row[0]);
    }
    return 0;
}
// 根据id找cfd
int Person::findCfd(int id)
{
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select cfd from userdata where id = '%d';", id);

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row != nullptr)
    {
        return atoi(row[0]);
    }
    return 0;
}
// 是自己
int Person::isMyself() // msg.id是传过来的好友id，findId是根据cfd对应出来的id
{
    std::cout << msg.id << " " << findId() << std::endl;
    if (msg.id == findId())
    {
        return 1;
    }
    return 0;
}
// 已经是好朋友啦
int Person::isFriend() // msg.id是传过来的好友id，findId是根据cfd对应出来的id，查看他俩是否已经在数据库中
{
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select 1 from friendlist where (usera = '%d' and userb = '%d')or (usera = '%d' and userb = '%d');", findId(), msg.id, msg.id, findId());

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    bool exists = (mysql_fetch_row(result) != nullptr);
    mysql_free_result(result);
    return exists;
}
// 添加好友
int Person::sq_addFriend() // 先看是否数据库有两者的消息，
{
    std::cout << "加好友的信息传进数据库" << std::endl;
    int sender = findId(); // 发送者的id，因为发送者肯定处于上现，id和cfd绑定在一起
    struct protocol msgback;
    // 先判断是否发过请求
    // status=0表示发的请求，1表示同意，2表示拒绝，3表示信息
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select status from  datamessage where (inid = %d and  toid = %d);", sender, msg.id);
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row != nullptr) // 表示有
    {
        if (atoi(row[0]) == 0)
            msgback.data = "已经发过请求了,等待对方验证";
        if (atoi(row[0]) == 2)
            msgback.data = "对方拒绝了你的请求";
        msgback.state = NOCONTINUE;
        send_data(msgback, sockfd);
        return 0;
    }
    // 现存在数据库中
    char sql_cmd1[256];
    snprintf(sql_cmd1, sizeof(sql_cmd1), "insert into datamessage(inid, toid,message,status) values('%d','%d','%s','%d');", findId(), msg.id, msg.data.c_str(), 0);
    ret = mysql_query(mysql, sql_cmd1);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    else
    {
        printf("[%d]对[%d]好友请求存入数据库\n", sender, msg.id);
    }
    if (checkUserOnline(msg.id))
    {
        msgback.id = findId();
        msgback.data = "发来加好友请求";
        msgback.state = REQUEST;
        send_data(msgback, findCfd(msg.id));
    }
    return 1;
}
// 屏蔽好友
int Person::sq_blockFriend() // msg.id是传过来的好友id，findId是根据cfd对应出来的id，设为0表示屏蔽
{
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "update friendlist set status=%d where (usera = '%d' and userb = '%d')or (usera = '%d' and userb = '%d');", 0, findId(), msg.id, msg.id, findId());

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql update error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    return 0;
}
// 删除好友
int Person::sq_deleteFriend() // msg.id是传过来的好友id，findId是根据cfd对应出来的自己id
{
    std::cout << msg.name << msg.data << std::endl;
    char sql_cmd[256];
    std::cout << msg.name << msg.data << std::endl;
    std::cout << sockfd << std::endl;
    snprintf(sql_cmd, sizeof(sql_cmd), "delete from friendlist where (usera = '%d' and userb = '%d')or (usera = '%d' and userb = '%d');", findId(), msg.id, msg.id, findId());

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql delete error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    // 然后还要删除关于他们所有在datamessage里面的聊天记录,不然删了在加他们就加不上了！！！
    char sql_cmd1[256];
    snprintf(sql_cmd1, sizeof(sql_cmd1), "delete from datamessage where (inid = '%d' and toid = '%d')or (inid = '%d' and toid = '%d');", findId(), msg.id, msg.id, findId());

    ret = mysql_query(mysql, sql_cmd1);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql delete error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    return 0;
}
// 根据id查好友的名字和状态
int Person::sq_statusFriend(int id)
{
    struct protocol msg_back;
    msg_back.cmd = STATUSFRIEND;
    std::cout << "好友的id为:" << id << std::endl;
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select username,status from userdata where id = '%d';", id);
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql query error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    if (result == NULL)
    {
        std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }

    int num_rows = mysql_num_rows(result);
    std::cout << "Number of rows: " << num_rows << std::endl; // 这我试了下，就是当好友注销了，但在friendlist表里还有记录，在userdata里查不到不会报错
    for (int i = 0; i < num_rows; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row == NULL)
        {
            std::cerr << "[ERR] mysql fetch row error: " << mysql_error(mysql) << std::endl;
            break;
        }
        msg_back.name = row[0];
        msg_back.data = row[1]; // data里面存的是好友的status
        msg_back.id = id;
        msg_back.state = STATUSFRIEND_OK; // 表示好友状态显示还没结束
        send_data(msg_back, sockfd);
        std::cout << "好友名字: " << row[0] << std::endl;
        std::cout << "好友状态: " << row[1] << std::endl;
    }

    mysql_free_result(result);
    return 0;
}
// 先将发送的消息存起来
int Person::sq_restoreFriend()
{
    std::cout << msg.data << std::endl;
    // 终于找到mysql为啥存不进去的原因，给设的缓冲区太小了
    char sql_cmd[10000];
    snprintf(sql_cmd, sizeof(sql_cmd), "insert into datamessage (inid,toid,status,message) values('%d','%d','%d','%s')", findId(), msg.id, 3, msg.data.c_str());

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    else
        printf("[%d]发给[%d]聊天记录成功录入数据库\n", findId(), msg.id);
    return 0;
}
// 将存储在数据库的信息发送给好友
// int Person::senddatatoFriend()
// {
//     char sql_cmd[256];
//     snprintf(sql_cmd, sizeof(sql_cmd), "select  * from  messagedata where (inid = %d and  toid = %d and status = %d)", findId(), msg.id, 3); // 3未发送
//     int ret = mysql_query(mysql, sql_cmd);
//     if (ret != 0)
//     {
//         std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
//         return -1; // Error indicator
//     }
//     MYSQL_RES *result = mysql_store_result(mysql);
//     MYSQL_ROW row;
//     // 循环读取每一行数据
//     while (row = mysql_fetch_row(result))
//     {
//         char sql_cmd1[256];
//         snprintf(sql_cmd1, sizeof(sql_cmd1), "update messagedata set status = %d where (inid = %d and toid = %d and id=%d)", 4, findId(), msg.id, atoi(row[4])); // 4已经发送
//         int ret = mysql_query(mysql, sql_cmd1);
//         if (ret != 0)
//         {
//             std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
//             return -1; // Error indicator
//         }
//         else
//         {
//             struct protocol msg_back;
//             msg_back.id = findId();
//             msg_back.state = OP_OK;
//             msg_back.data = row[2];
//             send_data(msg_back, findCfd());
//             printf("[%d]对[%d]消息发送成功\n", findId(), msg.id);
//         }
//     }
//     return 0;
// }
int Person::sq_unblockFriend() // 解除屏蔽好友
{
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "update friendlist set status = %d where (usera = %d and userb = %d) or (usera = %d and userb = %d)", 1, findId(), msg.id, msg.id, findId());
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql update error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    return 0;
}
// 下线
void Person::offline()
{
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "update userdata set status='%d',cfd ='%d' where cfd='%d';", 0, -1, sockfd);

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql update error: " << mysql_error(mysql) << std::endl;
    }
    // 下线之后要把vector里面map对应的id删了，不然如果服务器一直不关，你每次登录一次，都会在把你相同的id加进vector里面，这样发消息的时候，可能给他发了好几次
    // 新加的
    for (auto it = mang.begin(); it != mang.end();)
    {
        // 遍历每个键值对
        bool found = false; // 标记是否找到了要删除的元素

        for (auto &pair : *it)
        {
            if (pair.first == findId())
            {
                found = true; // 找到了匹配的条目
                break;        // 退出内部循环
            }
        }

        if (found)
        {
            it = mang.erase(it); // 删除当前元素，并更新迭代器
        }
        else
        {
            ++it; // 移动到下一个元素
        }
    }
}
// 上线
void Person::upline() // 根据id将状态改为1,cfd改为当前套接字
{
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "update userdata set status=%d ,cfd=%d where id='%d';", 1, sockfd, msg.id);

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql update error: " << mysql_error(mysql) << std::endl;
    }
}

// 注册
void Person::registerUser()
{
    struct protocol msg_back;
    msg_back.cmd = REGISTE;
    msg_back.id = addUser();
    printf("%d\n", sockfd);
    std::cout << msg_back.cmd << std::endl;
    msg_back.state = OP_OK;
    // 发给客户端
    send_data(msg_back, sockfd);
    // 加这个是为了判断服务器是不是在发文件。在发就是1，不发就是0
    std::map<int, int> map1;
    map1[msg_back.id] = 0;
    mang.push_back(map1);
    for (const auto &group : mang)
    {
        for (const auto &pair : group)
        {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
        std::cout << std::endl;
    }
    return;
}

// 用户是否存在
bool Person::checkUserExists() // 找userdata中有没有这个id，有则返回1
{
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select 1 from userdata where id = '%d';", msg.id);

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql query error: " << mysql_error(mysql) << std::endl;
        return false;
    }

    MYSQL_RES *result = mysql_store_result(mysql);
    bool exists = (mysql_fetch_row(result) != nullptr);
    mysql_free_result(result);
    return exists;
}
// 查找想屏蔽的好友是否已经屏蔽
int Person::sq_isBlocked() // msg.id为想屏蔽的好友id，findId()为当前用户id
{
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select status from friendlist where (usera = %d and userb = %d) or (usera = %d and userb = %d);", findId(), msg.id, msg.id, findId());
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
        return false;
    }

    MYSQL_RES *result = mysql_store_result(mysql);
    bool isblock = false;
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row != nullptr)
    {
        isblock = (atoi(row[0]) == 0); // 返回1表示已经屏蔽，0表示未屏蔽
    }
    mysql_free_result(result);
    return isblock;
}
// 用户是否已经在线
bool Person::checkUserOnline(int id) // 检查id对应的状态是否为1，为1则已经在线(id->status)
{
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select status from userdata where id ='%d';", id);

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql query error: " << mysql_error(mysql) << std::endl;
        return false;
    }

    MYSQL_RES *result = mysql_store_result(mysql);
    bool online = false;
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row != nullptr)
    {
        online = (atoi(row[0]) == 1);
    }
    mysql_free_result(result);
    return online;
}

// 检查密码是否正确
bool Person::checkUserPassword()
{
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select password from userdata where id ='%d';", msg.id);

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql query error: " << mysql_error(mysql) << std::endl;
        return false;
    }

    MYSQL_RES *result = mysql_store_result(mysql);
    bool password_correct = false;
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row != nullptr)
    {
        password_correct = (strcmp(row[0], msg.data.c_str()) == 0);
    }
    mysql_free_result(result);
    return password_correct;
}

// 登录
void Person::loginUser() // 先看用户是否存在，然后看用户是否已经在线，再判断密码是否正确，最后上线
{
    std::map<int, int> map1;
    map1[msg.id] = 0;
    mang.push_back(map1);

    struct protocol msg_back;
    msg_back.cmd = LOGIN;

    if (checkUserExists())
    {
        if (checkUserOnline(msg.id))
        {
            msg_back.state = USER_LOGED;
        }
        else
        {
            if (checkUserPassword())
            {
                upline();
                msg_back.state = OP_OK;
            }
            else
            {
                msg_back.state = PASSWORD_ERROR;
            }
        }
    }
    else
    {
        msg_back.state = USER_NOT_REGIST;
    }
    send_data(msg_back, sockfd);
    for (const auto &group : mang)
    {
        for (const auto &pair : group)
        {
            std::cout << pair.first << ": " << pair.second << std::endl;
        }
        std::cout << std::endl;
    }
}
// 注销
void Person::logoffUser()
{
    std::cout << "logoff" << std::endl;
    struct protocol msg_back;
    msg_back.cmd = LOGOFF;

    if (checkUserExists())
    {
        if (checkUserOnline(msg.id))
        {
            msg_back.state = USER_LOGED;
        }
        else if (checkUserPassword())
        {
            deleteUser();
            msg_back.state = OP_OK;
        }
        else
        {
            msg_back.state = PASSWORD_ERROR;
        }
    }
    else
    {
        msg_back.state = USER_NOT_REGIST;
    }
    send_data(msg_back, sockfd);
}
// 是否接受好友请求
int Person::ynAccept()
{
    struct protocol msg_back;
    msg_back.id = findId();
    if (msg.data == "Y")
    {
        char sql_cmd[256];
        std::cout << msg.id << std::endl;
        std::cout << findId() << std::endl;
        snprintf(sql_cmd, sizeof(sql_cmd), "update datamessage set status=%d where(inid=%d and toid=%d and status=%d);", 1, msg.id, findId(), 0); // 同意好友请求
        int ret = mysql_query(mysql, sql_cmd);
        if (ret != 0)
        {
            std::cerr << "[ERR] mysql update error: " << mysql_error(mysql) << std::endl;
            return false;
        }

        char sql_cmd1[256];
        snprintf(sql_cmd1, sizeof(sql_cmd1), "insert into friendlist(usera,userb,status) values(%d,%d,1);", msg.id, findId()); // 添加好友
        ret = mysql_query(mysql, sql_cmd1);
        if (ret != 0)
        {
            std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
            return false;
        }
        msg_back.state = AGREEFRIENDNEED;
    }
    else
    {
        // char sql_cmd2[256];
        // snprintf(sql_cmd2, sizeof(sql_cmd2), "update datamessage set status=%d where(inid=%d and toid=%d and status=%d);", 2, msg.id, findId(), 0); // 拒绝好友请求
        // int ret = mysql_query(mysql, sql_cmd2);
        // if (ret != 0)
        // {
        //     std::cerr << "[ERR] mysql updata error: " << mysql_error(mysql) << std::endl;
        //     return false;
        // }
        char sql_cmd2[256];
        snprintf(sql_cmd2, sizeof(sql_cmd2), "delete from datamessage where(inid=%d and toid=%d and status=%d);", msg.id, findId(), 0); // 拒绝好友请求
        int ret = mysql_query(mysql, sql_cmd2);
        if (ret != 0)
        {
            std::cerr << "[ERR] mysql delete error: " << mysql_error(mysql) << std::endl;
            return false;
        }
        msg_back.state = REFUSEFRIENDNEED;
    }
    send_data(msg_back, findCfd(msg.id));
    return 0;
}
void Person::addFriend() // 添加好友//先看好友是否存在，在看是否加的是自己，然后看是否已经都成为好朋友，最后将申请写进数据库
{
    struct protocol msg_back;
    msg_back.cmd = ADDFRIEND;

    if (checkUserExists())
    {
        if (isMyself())
        {
            msg_back.state = MYSELF; // 是自己
        }
        else if (isFriend())
        {
            msg_back.state = ISFRIEND; // 已经是好友
        }
        else
        {
            // 在这应该在判断一下，他是否已经给你发过好友申请了，如果已经发过了，就不需要再发了，只需要在好友通知里面同意一下就可以了
            char sql_cmd[256];
            snprintf(sql_cmd, sizeof(sql_cmd), "select 1 from datamessage where (inid = '%d' and toid='%d' and status='%d');", msg.id, findId(), 0); // 查询是否已经发送过好友请求

            int ret = mysql_query(mysql, sql_cmd);
            if (ret != 0)
            {
                std::cerr << "[ERR] mysql query error: " << mysql_error(mysql) << std::endl;
                return;
            }

            MYSQL_RES *result = mysql_store_result(mysql);
            bool exists = (mysql_fetch_row(result) != nullptr);
            mysql_free_result(result);
            if (exists) // 证明你想加的那个人已经给你发过好友请求了，你只需要同意一下就可以了
            {
                msg_back.state = OLDSEND;
            }
            else
            {
                if (sq_addFriend())
                    msg_back.state = OP_OK;
            }
        }
    }
    else
    {
        msg_back.state = USER_NOT_REGIST;
    }
    send_data(msg_back, sockfd);
}
// 屏蔽好友//这有问题，要是屏蔽了两个一起相互私聊，收不到对方法的消息！！还没改
void Person::blockFriend() // 先看好友是否存在，在看是否加的是自己，然后看是否还不是朋友，如果是朋友，是否已经屏蔽过了
{
    struct protocol msg_back;
    msg_back.cmd = BLOCKFRIEND;

    if (checkUserExists())
    {
        if (isMyself())
        {
            msg_back.state = MYSELF; // 是自己
        }
        else if (isFriend())
        {
            if (!sq_isBlocked()) // 看是否已经屏蔽
            {
                sq_blockFriend();
                msg_back.state = OP_OK; // 屏蔽好友成功
            }
            else
            {
                msg_back.state = ISBLOCKED; // 已经屏蔽好友
            }
        }
        else
        {
            msg_back.state = NOTFRIEND; // 不是好友
        }
    }
    else
    {
        msg_back.state = USER_NOT_REGIST;
    }
    send_data(msg_back, sockfd);
}
void Person::deleteFriend() // 删除好友//先看好友是否存在，在看是否删的是自己，然后看是否还不是朋友，如果是朋友，可以删了
{
    struct protocol msg_back;
    msg_back.cmd = BLOCKFRIEND;

    if (checkUserExists())
    {
        if (isMyself())
        {
            msg_back.state = MYSELF; // 是自己
        }
        else if (isFriend())
        {
            sq_deleteFriend();
            msg_back.state = OP_OK; // 删除好友成功
        }
        else
        {
            msg_back.state = NOTFRIEND; // 不是好友
        }
    }
    else
    {
        msg_back.state = USER_NOT_REGIST;
    }
    send_data(msg_back, sockfd);
}
void Person::statusFriend() // 查看好友状态//先在数据库中找到自己的好友id,在根据id去查状态
{
    struct protocol msg_back;
    msg_back.cmd = STATUSFRIEND;
    std::vector<int> friendid;
    std::cout << "status" << std::endl;
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select usera from friendlist where userb='%d' union select userb from friendlist where usera='%d';", findId(), findId());

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    if (result == NULL)
    {
        std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
        return;
    }

    int num_rows = mysql_num_rows(result);
    for (int i = 0; i < num_rows; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row == NULL)
        {
            std::cerr << "[ERR] mysql fetch row error: " << mysql_error(mysql) << std::endl;
            break;
        }
        friendid.push_back(atoi(row[0]));
        std::cout << "friend: " << friendid[i] << std::endl;
        //    sq_statusFriend(atoi(row[0]));
    }

    mysql_free_result(result);
    for (int i = 0; i < friendid.size(); i++)
    {
        sq_statusFriend(friendid[i]);
    }
    msg_back.state = OP_OK; // 好友状态已经发完
    send_data(msg_back, sockfd);
    return;
}
void Person::privateChat() // 私聊//好友是否存在，是否是自己，是否是好友，是否在线，在线则发送消息
{
    std::cout << "privateChat" << std::endl;
    int flag = 1;
    struct protocol msg_back1, msg_back2;
    msg_back1.cmd = PRIVATECHAT;

    if (checkUserExists())
    {
        if (isMyself())
        {
            msg_back1.state = MYSELF; // 是自己
        }
        else if (isFriend())
        {
            if (msg.state == CHATFRIENDRECORD_OK)
            {
                std::cout << "chatfriendrecord" << std::endl;
                sq_chatfriendRecord();
            }
            if (msg.state == OP_OK)
            {
                std::cout << "chatfriend" << std::endl;
                sq_restoreFriend();
                if (checkUserOnline(msg.id)) // 在线的话看是否屏蔽，没屏蔽就可以直接收到消息
                {
                    flag = 0;
                    if (!sq_isBlocked()) // 如果没有屏蔽，直接发给他
                    {
                        // 新加的
                        for (auto &map : mang)
                        {
                            // 遍历每个键值对
                            for (auto &pair : map)
                            {

                                if (pair.first == msg.id)
                                {
                                    std::cout << "find" << std::endl;

                                    if (pair.second == 0)
                                    {

                                        msg_back2.state = YNCHAT;
                                        msg_back2.id = findId(); // 发送者id
                                        msg_back2.data = msg.data;
                                        send_data(msg_back2, findCfd(msg.id)); // 发送消息
                                    }
                                }
                            }
                        }
                    }
                }
                else
                    msg_back1.state = FRIEND_OFFLINE; // 对方不在线
            }
        }
        else
        {
            msg_back1.state = NOTFRIEND; // 不是好友
        }
    }
    else
    {
        msg_back1.state = USER_NOT_REGIST;
        std::cout << "user not regist" << std::endl;
    }
    if (flag)
    {

        send_data(msg_back1, sockfd); // 发送消息
        std::cout << "别放弃！" << std::endl;
    }
}
void Person::unblockFriend() // 取消屏蔽好友
{
    struct protocol msg_back;
    msg_back.cmd = UNBLOCKFRIEND;

    if (checkUserExists())
    {
        if (isMyself())
        {
            msg_back.state = MYSELF; // 是自己
        }
        else if (isFriend())
        {
            if (sq_isBlocked())
            {
                sq_unblockFriend();
                msg_back.state = OP_OK; // 解除屏蔽好友成功
            }
            else
            {
                msg_back.state = ISUNBLOCKED; // 已经屏蔽该好友没有屏蔽该好友
            }
        }
        else
        {
            msg_back.state = NOTFRIEND; // 不是好友
        }
    }
    else
    {
        msg_back.state = USER_NOT_REGIST;
    }
    send_data(msg_back, sockfd);
}
int Person::friendNotice() // 好友通知
{
    int id = findId();
    struct protocol msg_back;
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select inid,message from datamessage where (toid = '%d' and status ='%d');", id, 0);
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    if (result == NULL)
    {
        std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    int num_rows = mysql_num_rows(result);
    for (int i = 0; i < num_rows; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row == NULL)
        {
            std::cerr << "[ERR] mysql fetch row error: " << mysql_error(mysql) << std::endl;
            break;
        }
        std::cout << atoi(row[0]) << ":" << row[1] << std::endl;
        msg_back.id = atoi(row[0]);
        msg_back.data = row[1];
        send_data(msg_back, sockfd);
        std::cout << "hahaha" << std::endl;
    }
    msg_back.state = OP_OK;
    send_data(msg_back, sockfd);
    mysql_free_result(result);
    return 0;
}
int Person::sq_chatfriendRecord() // 好友聊天记录,msg.id是好友id
{
    int myid = findId(); // 自己id
    int friendid = msg.id;
    struct protocol msg_back;
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select * from datamessage where (toid = '%d' and inid='%d' and status ='%d')or (toid = '%d' and inid='%d' and status ='%d');", friendid, myid, 3, myid, friendid, 3);
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    if (result == NULL)
    {
        std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    int num_rows = mysql_num_rows(result);
    for (int i = 0; i < num_rows; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row == NULL)
        {
            std::cerr << "[ERR] mysql fetch row error: " << mysql_error(mysql) << std::endl;
            break;
        }
        std::cout << atoi(row[0]) << "->" << atoi(row[1]) << row[2] << std::endl;
        msg_back.state = CHATFRIENDRECORD_OK;
        msg_back.id = atoi(row[0]);
        msg_back.data = row[2];
        send_data(msg_back, sockfd);
    }
    mysql_free_result(result);
    return 0;
}
void Person::chatfriendRecord() // 好友聊天记录//好友是否存在，是否是自己，是否是好友
{
    struct protocol msg_back;
    msg_back.cmd = CHATFRIENDRECORD;
    if (checkUserExists())
    {
        if (isMyself())
        {
            msg_back.state = MYSELF; // 是自己
        }
        if (isFriend())
        {
            sq_chatfriendRecord();
            msg_back.state = OP_OK;
        }
        else
        {
            msg_back.state = NOTFRIEND; // 还不是是好友
        }
    }
    else
    {
        msg_back.state = USER_NOT_REGIST;
    }
    send_data(msg_back, sockfd);
}
void Person::createGroup() // 创建群聊
{
    struct protocol msg_back;
    msg_back.cmd = CREATEGROUP;
    if (!checkGroup())
    {
        msg_back.state = OP_OK;
        char sql_cmd[256];
        std::cout << msg.name << std::endl;
        std::cout << sockfd << std::endl;
        snprintf(sql_cmd, sizeof(sql_cmd), "insert into groupdata(name,userid,type) values('%s','%d','%d');",
                 msg.name.c_str(), findId(), 1);

        int ret = mysql_query(mysql, sql_cmd);
        if (ret != 0)
        {
            std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
        }
    }
    // 发给客户端
    send_data(msg_back, sockfd);
    return;
}
int Person::checkGroup() // 群是否存
{
    std::cout << "checkGroup" << std::endl;
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select * from groupdata where name='%s';", msg.name.c_str());

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql query error: " << mysql_error(mysql) << std::endl;
        return 0;
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    if (result == NULL) // 这个只是收集结果，还需判断是否有行
    {
        std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
    }
    if (mysql_num_rows(result) == 0)
    {
        mysql_free_result(result); // 释放结果集占用的内存
        return 0;                  // 没有找到群组
    }

    mysql_free_result(result); // 释放结果集占用的内存
    return 1;                  // 找到群组
}
int Person::groupynMe(int id) // 群中是否有我
{
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select type from groupdata where (userid='%d' and name='%s');", id, msg.name.c_str());
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    MYSQL_ROW row = mysql_fetch_row(result);
    if (row != NULL)
    {
        return atoi(row[0]);
    }
    return 0;
}
void Person::deleteGroup() // 退出/ 删除群聊//群是否存在,群里是否不存在这个人，是否是群主
{
    std::cout << findId() << std::endl;

    std::cout << msg.name.c_str() << std::endl;
    struct protocol msg_back;
    msg_back.cmd = DELETEGROUP;
    if (checkGroup())
    {
        if (groupynMe(findId()))
        {
            msg_back.state = OP_OK;
            if (groupynMe(findId()) == 1) // 是群主，这是新加的，之前没想到，将群一删,将群里面的消息都得一删
            {
                // 从groupdata里面先一删
                msg_back.cmd = LENDER;
                char sql_cmd[256];
                snprintf(sql_cmd, sizeof(sql_cmd), "delete from groupdata where name='%s';", msg.name.c_str());
                int ret = mysql_query(mysql, sql_cmd);
                if (ret != 0)
                {
                    std::cerr << "[ERR] mysql delete error: " << mysql_error(mysql) << std::endl;
                }
                // 在从groupmessage里面一删
                char sql_cmd1[256];
                snprintf(sql_cmd1, sizeof(sql_cmd1), "delete from groupmessage where name='%s';", msg.name.c_str());
                ret = mysql_query(mysql, sql_cmd1);
                if (ret != 0)
                {
                    std::cerr << "[ERR] mysql delete error: " << mysql_error(mysql) << std::endl;
                }
            }
            else // 不是群主，自己退出
            {
                msg_back.cmd = ORDINARY;
                char sql_cmd1[256];
                snprintf(sql_cmd1, sizeof(sql_cmd1), "delete from groupdata where (userid='%d' and name='%s');", findId(), msg.name.c_str());

                int ret = mysql_query(mysql, sql_cmd1);
                if (ret != 0)
                {
                    std::cerr << "[ERR] MySQL delete error: " << mysql_error(mysql) << std::endl;
                }
                // 还要删除群消息里面的自己，防止出现退出后加不进去的情况
                char sql_cmd2[256];
                snprintf(sql_cmd2, sizeof(sql_cmd2), "delete from groupmessage where (inid='%d' and name='%s');", findId(), msg.name.c_str());
                ret = mysql_query(mysql, sql_cmd2);
                if (ret != 0)
                {
                    std::cerr << "[ERR] MySQL delete error: " << mysql_error(mysql) << std::endl;
                }
            }
        }
        else
        {
            msg_back.state = GROUPNOTPERSON;
        }
    }
    else
    {
        msg_back.state = GROUP_NOT_EXIST;
    }
    send_data(msg_back, sockfd);
}
void Person::removeUser() // 先看群是否存在，你和要删除的人是否在群里面，删除群成员,群主可删除所有，管理员不能删除管理员，管理员可以删除普通成员,普通员工不可以删除任何人
{
    struct protocol msg_back;
    msg_back.cmd = REMOVEUSER;
    if (checkGroup())
    {
        std::cout << findId() << std::endl;
        std::cout << msg.id << std::endl;
        if (groupynMe(findId()) && groupynMe(msg.id)) // 自己和要删除的人都存在
        {
            std::cout << groupynMe(findId()) << std::endl;
            std::cout << groupynMe(msg.id) << std::endl;
            if (groupynMe(findId()) < groupynMe(msg.id)) // 自己权限大于要删除的人
            {
                msg_back.state = OP_OK;
                char sql_cmd[256];
                snprintf(sql_cmd, sizeof(sql_cmd), "delete from groupdata where (userid='%d' and name='%s');", msg.id, msg.name.c_str());
                int ret = mysql_query(mysql, sql_cmd);
                if (ret != 0)
                {
                    std::cerr << "[ERR] MySQL delete error: " << mysql_error(mysql) << std::endl;
                }
                // 新加的，删除了某个人之后，群消息里面也要删除，使得那个人可以继续进群
                char sql_cmd1[256];
                snprintf(sql_cmd1, sizeof(sql_cmd1), "delete from groupmessage where (inid='%d' and name='%s');", msg.id, msg.name.c_str());
                ret = mysql_query(mysql, sql_cmd1);
                if (ret != 0)
                {
                    std::cerr << "[ERR] MySQL delete error: " << mysql_error(mysql) << std::endl;
                }
            }
            else
            {
                msg_back.state = NOTRIGHT;
            }
        }
        else if (groupynMe(findId()) == 0) // 你不再群里面
        {
            msg_back.state = GROUPNOTPERSON;
        }
        else if (groupynMe(msg.id) == 0) // 要删除的人不在群里面
        {
            msg_back.state = GROUPNOTPERSON1;
        }
    }
    else
    {
        msg_back.state = GROUP_NOT_EXIST;
    }
    send_data(msg_back, sockfd);
}
void Person::sq_groupList()
{
    struct protocol msg_back1;
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select userid,type from groupdata where name='%s';", msg.name.c_str());
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    if (result == NULL)
    {
        std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
        return;
    }

    int num_rows = mysql_num_rows(result);
    for (int i = 0; i < num_rows; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row == NULL)
        {
            std::cerr << "[ERR] mysql fetch row error: " << mysql_error(mysql) << std::endl;
            break;
        }
        std::cout << "userid" << row[0] << "    " << "usertype" << row[1] << std::endl;
        msg_back1.id = atoi(row[0]);
        msg_back1.cmd = atoi(row[1]);
        msg_back1.state = GROUPLIST_OK;
        send_data(msg_back1, sockfd);
        std::cout << "userid" << row[0] << "    " << "usertype" << row[1] << std::endl;
    }

    mysql_free_result(result);
    return;
}
void Person::groupList() // 查看群成员,先看有没有这个群，然后再看你是不是群成员
{
    struct protocol msg_back;
    msg_back.cmd = GROUPLIST;
    if (checkGroup())
    {
        if (groupynMe(findId())) // 你在群里面
        {
            sq_groupList();
            msg_back.state = OP_OK;
        }
        else
        {
            msg_back.state = GROUPNOTPERSON;
        }
    }
    else
    {
        msg_back.state = GROUP_NOT_EXIST;
    }
    send_data(msg_back, sockfd);
}
void Person::iinGroup()
{
    struct protocol msg_back;
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select name from groupdata where userid='%d';", msg.id);
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    if (result == NULL)
    {
        std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
        return;
    }

    int num_rows = mysql_num_rows(result);
    // if (num_rows == 0)
    // {
    //     msg_back.state = NOGROUP;
    // }
    for (int i = 0; i < num_rows; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row == NULL)
        {
            std::cerr << "[ERR] mysql fetch row error: " << mysql_error(mysql) << std::endl;
            break;
        }
        std::cout << "name" << row[0] << std::endl;
        msg_back.state = IINGROUP_OK;
        msg_back.name = row[0];
        send_data(msg_back, sockfd);
    }
    msg_back.state = OP_OK;
    send_data(msg_back, sockfd);
    mysql_free_result(result);
}
void Person::sq_adManager()
{
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "update groupdata set type='%d' where userid='%d' and name='%s';", msg.state, msg.id, msg.name.c_str());
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql update error: " << mysql_error(mysql) << std::endl;
    }
    return;
}
void Person::adManager() // 这个群是否存在，你和他是否在里面，你是否是群主，你要加，他是否已经是管理员，你要删除，他是否已经不是管理员
{
    struct protocol msg_back;
    msg_back.cmd = ADMANAGER;
    if (checkGroup())
    {
        if (groupynMe(findId())) // 你在群里面
        {
            if (groupynMe(findId()) == 1) // 你是群主
            {
                if (groupynMe(msg.id)) // 你要加的人是群成员
                {
                    if(msg.id==findId()) // 你要加的人是群主
                    {
                        msg_back.state = MYSELF;
                    }
                    else if (msg.state == 2) // 你要加
                    {
                        if (groupynMe(msg.id) == 3) // 你要加的人不是管理员
                        {
                            sq_adManager();
                            msg_back.state = OP_OK;
                        }
                        else
                        {
                            msg_back.state = ISMANAGER;
                        }
                    }
                    else if (msg.state == 3) // 你要删除
                    {
                        if (groupynMe(msg.id) == 2) // 你要删除的人是管理员
                        {
                            sq_adManager();
                            msg_back.state = OP_OK;
                        }
                        else
                        {
                            msg_back.state = NOTMANAGER;
                        }
                    }
                }
                else
                {
                    msg_back.state = GROUPNOTPERSON1;
                }
            }
            else
            {
                msg_back.state = ISNOTLENDER;
            }
        }
        else
        {
            msg_back.state = GROUPNOTPERSON;
        }
    }
    else
    {
        msg_back.state = GROUP_NOT_EXIST;
    }
    send_data(msg_back, sockfd);
}
int Person::sq_applyaddGroup()
{
    int id = findId();
    std::cout << findId() << std::endl;
    struct protocol msgback;
    // 先判断是否已经发送过请求了
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select * from groupmessage where inid='%d' and name='%s';", id, msg.name.c_str());
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
        return -1;
    }
    MYSQL_RES *res = mysql_store_result(mysql);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (row != NULL)
    {
        msgback.data = "已经发过请求了,等待对方验证";
        msgback.state = NOCONTINUE;
        send_data(msgback, sockfd);
        return 0;
    }
    std::cout << findId() << std::endl;
    // 现在将请求存到数据库中
    char sql_cmd1[256];
    snprintf(sql_cmd1, sizeof(sql_cmd1), "insert into groupmessage (inid, name, message, status) values('%d','%s','%s','%d');", id, msg.name.c_str(), msg.data.c_str(), 0);
    ret = mysql_query(mysql, sql_cmd1);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
        return -1;
    }
    else
    {
        printf("[%d]对[%s]请求存入数据库\n", id, msg.name.c_str());
    }
    // 实现实时发送请求给群主和群管理员
    char sql_cmd2[256];
    snprintf(sql_cmd2, sizeof(sql_cmd2), "select  userid from groupdata where  (name ='%s' and  type='%d') or (name ='%s' and type='%d');", msg.name.c_str(), 1, msg.name.c_str(), 2); // 找到该群中该用户是否为管理员或群主
    int ret1 = mysql_query(mysql, sql_cmd2);
    if (ret1 != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
        return -1;
    }
    MYSQL_RES *result1 = mysql_store_result(mysql);
    if (result1 == NULL)
    {
        std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
        return -1;
    }
    int num_rows1 = mysql_num_rows(result1);
    std::cout << "nums" << num_rows1 << std::endl;
    for (int i = 0; i < num_rows1; i++)
    {
        row = mysql_fetch_row(result1);
        if (checkUserOnline(atoi(row[0])))
        {
            msgback.id = id;
            msgback.state = REQUEST;
            msgback.data = "想加入" + msg.name;
            send_data(msgback, findCfd(atoi(row[0])));
        }
    }
    return 0;
}
void Person::applyaddGroup() // 申请加群//群是否存在，你是否已经在群里面了
{
    // status=0表示发的请求，1表示同意，2表示拒绝，3表示信息
    std::cout << findId() << std::endl;
    struct protocol msg_back;
    msg_back.cmd = APPLYADDGROUP;
    if (checkGroup())
    {
        if (!groupynMe(findId())) // 你不在群里面
        {
            sq_applyaddGroup();
            msg_back.state = OP_OK;
        }
        else
        {
            msg_back.state = ISINGROUP;
        }
    }
    else
    {
        msg_back.state = GROUP_NOT_EXIST;
    }
    send_data(msg_back, sockfd);
}
int Person::ynacceptGroup()
{
    struct protocol msg_back;
    if (msg.data == "Y")
    {
        char sql_cmd[256];
        std::cout << msg.id << std::endl; // msg.id是发送请求的人的id
        msg_back.id = findId();           // 同意的那个人的id
        msg_back.name = msg.name;
        snprintf(sql_cmd, sizeof(sql_cmd), "update groupmessage set status=%d where(inid='%d' and name='%s' and status='%d');", 1, msg.id, msg.name.c_str(), 0); // 同意此人加入群聊
        int ret = mysql_query(mysql, sql_cmd);
        if (ret != 0)
        {
            std::cerr << "[ERR] mysql update error: " << mysql_error(mysql) << std::endl;
            return false;
        }

        char sql_cmd1[256];
        std::cout << "haha" << msg.id << std::endl;
        snprintf(sql_cmd1, sizeof(sql_cmd1), "insert into groupdata(name,userid,type) values('%s','%d','%d');", msg.name.c_str(), msg.id, 3); // 将此人加入群聊
        ret = mysql_query(mysql, sql_cmd1);
        if (ret != 0)
        {
            std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
            return false;
        }
        msg_back.state = AGREEGROUP;
    }
    else
    { // z怎么是datamessage呢，还浅浅发现了一个问题
        // char sql_cmd2[256];
        // snprintf(sql_cmd2, sizeof(sql_cmd2), "update datamessage set status=%d where(inid='%d' and name='%s' and status='%d');", 2, msg.id, msg.name.c_str(), 0); // 拒绝好友请求
        // int ret = mysql_query(mysql, sql_cmd2);
        // if (ret != 0)
        // {
        //     std::cerr << "[ERR] mysql updata error: " << mysql_error(mysql) << std::endl;
        //     return false;
        // }
        char sql_cmd2[256];
        snprintf(sql_cmd2, sizeof(sql_cmd2), "delete from groupmessage where (inid='%d' and name='%s' and status='%d');", msg.id, msg.name.c_str(), 0); // 拒绝群申请
        int ret = mysql_query(mysql, sql_cmd2);
        if (ret != 0)
        {
            std::cerr << "[ERR] mysql delete error: " << mysql_error(mysql) << std::endl;
            return false;
        }
        msg_back.id = findId(); // 同意的那个人的id
        msg_back.name = msg.name;
        msg_back.state = REFUSEGROUP;
    }
    if (checkUserOnline(msg.id))
        send_data(msg_back, findCfd(msg.id)); // 发给发送请求的人是否同意加群，哪得看他是否在线阿,上限才发
    return 0;
}
int Person::groupNotice()
{
    int id = findId();
    std::cout << id << std::endl;
    char sql_cmd[256];
    struct protocol msg_back;
    snprintf(sql_cmd, sizeof(sql_cmd), "select inid,message,name from groupmessage where  status ='%d';", 0); // 找到所有status=0的
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    if (result == NULL)
    {
        std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    int num_rows = mysql_num_rows(result);
    std::cout << "num_rows: " << num_rows << std::endl;
    for (int i = 0; i < num_rows; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row == NULL)
        {
            std::cerr << "[ERR] mysql fetch row error: " << mysql_error(mysql) << std::endl;
            return -1; // Error indicator
        }
        char sql_cmd1[256];
        snprintf(sql_cmd1, sizeof(sql_cmd1), "select *from groupdata where  (name ='%s' and userid='%d' and type='%d') or (name ='%s' and userid='%d' and type='%d');", row[2], id, 1, row[2], id, 2); // 找到该群中该用户是否为管理员或群主
        int ret1 = mysql_query(mysql, sql_cmd1);
        if (ret1 != 0)
        {
            std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
            return -1;
        }
        MYSQL_RES *result1 = mysql_store_result(mysql);
        if (result1 == NULL)
        {
            std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
            return -1;
        }
        int num_rows1 = mysql_num_rows(result1); // 没在这个群中或者不是管理员或群主
        if (num_rows1 == 0)
        {

            continue;
        }
        else
        {
            msg_back.id = atoi(row[0]);
            msg_back.data = row[1];
            msg_back.name = row[2]; // 群名
            send_data(msg_back, findCfd(msg.id));
        }
    }
    msg_back.state = OP_OK;
    send_data(msg_back, findCfd(msg.id));
    return 0;
}
int Person::sq_restoreGroup()
{
    char sql_cmd[10000];
    snprintf(sql_cmd, sizeof(sql_cmd), "insert into groupmessage (inid,name,status,message) values('%d','%s','%d','%s')", findId(), msg.name.c_str(), 3, msg.data.c_str());

    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql insert error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    else
        printf("[%d]发给[%s]聊天记录成功录入数据库\n", findId(), msg.name.c_str());
    return 0;
}
int Person::groupChat() // 群聊//看这个群是否存在，你是否在群里面
{
    int flag = 1;
    struct protocol msg_back;
    if (checkGroup())
    {
        if (groupynMe(findId()))
        {
            if (msg.state == CHATGROUPRECORD_OK)
            {
                groupchatRecord();
            }
            if (msg.state == OP_OK)
            {

                flag = 0;
                sq_restoreGroup(); // 存在数据库
                // 找群里的每一个人，如果在线，就发给他
                char sql_cmd[256];
                snprintf(sql_cmd, sizeof(sql_cmd), "select  userid from groupdata where name='%s';", msg.name.c_str());
                int ret = mysql_query(mysql, sql_cmd);
                if (ret != 0)
                {

                    std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
                    return -1; // Error indicator
                }
                MYSQL_RES *result = mysql_store_result(mysql);
                if (result == NULL)
                {
                    std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
                    return -1; // Error indicator
                }
                int num_rows = mysql_num_rows(result);
                for (int i = 0; i < num_rows; i++)
                {
                    MYSQL_ROW row = mysql_fetch_row(result);
                    if (row == NULL)
                    {
                        std::cerr << "[ERR] mysql fetch row error: " << mysql_error(mysql) << std::endl;
                        return -1; // Error indicator
                    }
                    if (atoi(row[0]) == findId())
                    {
                        continue; // 自己不用发给自己
                    }
                    else
                    {
                        if (checkUserOnline(atoi(row[0]))) // 看用户是否在线
                        {
                            // 新加的
                            for (auto &map : mang)
                            {
                                // 遍历每个键值对
                                for (auto &pair : map)
                                {

                                    if (pair.first == atoi(row[0]))
                                    {
                                        std::cout << "find" << std::endl;

                                        if (pair.second == 0)
                                        {
                                            msg_back.state = YNGROUPCHAT;
                                            msg_back.id = findId();
                                            msg_back.data = msg.data;
                                            msg_back.name = msg.name;
                                            send_data(msg_back, findCfd(atoi(row[0])));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            msg_back.state = GROUPNOTPERSON;
        }
    }
    else
    {
        msg_back.state = GROUP_NOT_EXIST;
    }
    if (flag)
    {
        send_data(msg_back, findCfd(findId()));
    }
    return 0;
}
int Person::sq_chatgroupRecord()
{
    struct protocol msg_back;
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select * from groupmessage where name='%s';", msg.name.c_str());
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    if (result == NULL)
    {
        std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    int num_rows = mysql_num_rows(result);
    for (int i = 0; i < num_rows; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row == NULL)
        {
            std::cerr << "[ERR] mysql fetch row error: " << mysql_error(mysql) << std::endl;
            break;
        }
        std::cout << atoi(row[0]) << "在" << atoi(row[1]) << "群里说" << row[2] << std::endl;
        msg_back.state = CHATGROUPRECORD_OK;
        msg_back.id = atoi(row[0]);
        msg_back.name = row[1];
        msg_back.data = row[2];
        send_data(msg_back, sockfd);
    }
    mysql_free_result(result);
    return 0;
}
void Person::groupchatRecord()
{
    struct protocol msg_back;
    if (checkGroup())
    {
        if (groupynMe(findId()))
        {
            sq_chatgroupRecord();
            msg_back.state = OP_OK;
        }
        else
        {
            msg_back.state = GROUPNOTPERSON;
        }
    }
    else
    {
        msg_back.state = GROUP_NOT_EXIST;
    }
    send_data(msg_back, sockfd);
}
void Person::checkFile() // 先看是给群里发还是给个人发，然后看这个群是否存在，你是否在群里面，这个人是否存在，你是否和他是好友
{
    struct protocol msg_back;
    if (msg.id != 0) // 表示是给好友发
    {
        if (checkUserExists())
        {
            if (isFriend())
            {
                msg_back.state = OP_OK;
            }
            else
            {
                msg_back.state = NOTFRIEND;
            }
        }
        else
        {
            msg_back.state = USER_NOT_REGIST;
        }
    }
    if (msg.id == 0) // 表示是给群发
    {
        if (checkGroup()) // 检查这个群是否存在
        {
            if (groupynMe(findId())) // 群里面是否有我
            {
                msg_back.state = OP_OK;
            }
            else
            {
                msg_back.state = GROUPNOTPERSON;
            }
        }
        else
        {
            msg_back.state = GROUP_NOT_EXIST;
        }
    }
    send_data(msg_back, sockfd);
    return;

    // msg_back.filename = msg.filename + ".sever";

    // if (msg.state == SENDFILEEND) // 文件全部接受完毕
    // {
    //     msg_back.state = OP_OK;
    //     send_data(msg_back, sockfd);
    //     // 还需要判断是群还是好友，然后进行不同的操作
    //     if (msg.id != 0) // 表示是给好友发
    //     {
    //         if (checkUserOnline(msg.id))
    //         {
    //             msg_back.id = findId();
    //             msg_back.state = REQUEST;
    //             msg_back.data = "给你发了一个文件" + msg_back.filename;
    //             send_data(msg_back, findCfd(msg.id));
    //         }
    //     }
    //     if (msg.id == 0) // 表示是给群发
    //     {
    //         // 先寻找群里的每一个成员
    //         char sql_cmd[256];
    //         snprintf(sql_cmd, sizeof(sql_cmd), "select userid from groupdata where name='%s';", msg.name.c_str());
    //         int ret = mysql_query(mysql, sql_cmd);
    //         if (ret != 0)
    //         {
    //             std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
    //         }
    //         MYSQL_RES *result = mysql_store_result(mysql);
    //         if (result == NULL)
    //         {
    //             std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
    //             return;
    //         }

    //         int num_rows = mysql_num_rows(result);
    //         for (int i = 0; i < num_rows; i++)
    //         {
    //             MYSQL_ROW row = mysql_fetch_row(result);
    //             if (row == NULL)
    //             {
    //                 std::cerr << "[ERR] mysql fetch row error: " << mysql_error(mysql) << std::endl;
    //                 break;
    //             }
    //             if(atoi(row[0]) == findId())//自己不用发
    //                 continue;
    //             if (checkUserOnline(atoi(row[0])))
    //             {

    //                 msg_back.id = findId();
    //                 msg_back.state = REQUEST;
    //                 msg_back.data = "给你发了一个文件" + msg_back.filename;
    //                 send_data(msg_back, findCfd(msg.id));
    //             }
    //         }
    //     }
    // }
    //     else
    //     {
    //         //std::ios::binary |
    //         std::ofstream f;
    //         f.open(msg_back.filename,  std::ios::app); // 打开文件进行追加
    //         f.seekp(msg.state, f.beg);
    //         //  std::cout<<msg.data<<std::endl;
    //         f.write(msg.data.c_str(), msg.data.length());
    //         f.close();
    //         msg_back.state = SENDFILE_OK;
    //         send_data(msg_back, sockfd);
    //     }
}
void Person::sendFile()
{
    std::string filename;
    // 在服务端创建一个,给思路之后丢给chat
    std::filesystem::path dirPath = "/home/sweet/sever/";
    if (!std::filesystem::exists(dirPath))
    {
        try
        {
            std::filesystem::create_directories(dirPath);
            std::cout << "Directory created successfully" << std::endl;
        }
        catch (const std::filesystem::filesystem_error &e)
        {
            std::cerr << "Error creating directory: " << e.what() << std::endl;
        }
    }
    else
    {
        std::cout << "Directory already exists" << std::endl;
    }
    filename = dirPath.c_str() + msg.filename;
    FILE *fp = fopen(filename.c_str(), "wb");
    if (fp == NULL)
    {
        std::cerr << "Failed to open file for writing" << std::endl;
        return;
    }

    // int original_flags = fcntl(sockfd, F_GETFL, 0);

    // if (fcntl(sockfd, F_SETFL, original_flags & ~O_NONBLOCK) == -1)
    // {
    //     std::cerr << "Failed to set file descriptor to blocking mode: " << strerror(errno) << std::endl;
    //     fclose(fp);
    //     return;
    // }
    int len;
    char buffer[10240];
    off_t total_received = 0;

    while (total_received < msg.filesize)
    {
        len = recv(sockfd, buffer, sizeof(buffer), 0);
        if (len <= 0)
        {
            if (len < 0)
            {
                perror("recv");
            }
            return;
        }

        fwrite(buffer, 1, len, fp);
        total_received += len;
        std::cout << "\33[2K\r" << msg.filename << ": " << (int)(((float)total_received / msg.filesize) * 100) << "%" << std::flush;
    }
    std::cout << std::endl;
    fclose(fp);
    if (total_received == msg.filesize)
    {
        std::cout << "File received successfully" << std::endl;
        struct protocol msg_back1;
        msg_back1.state = SENDFILE_OK;
        send_data(msg_back1, sockfd);
        std::cout << "bbb" << sockfd << std::endl;
    }
    else
    {
        std::cerr << "File size mismatch" << std::endl;
    }

    struct protocol msg_back;
    msg_back.id = findId();
    msg_back.state = REQUEST;
    // msg_back.data = "在群“+msg.name+”发了一个文件" + filename;
    if (msg.id == 0) // 表示是给群发
    {
        msg_back.data = "在群" + msg.name + "发了一个文件" + msg.filename;
        // 先寻找群里的每一个成员
        char sql_cmd[256];
        snprintf(sql_cmd, sizeof(sql_cmd), "select userid from groupdata where name='%s';", msg.name.c_str());
        int ret = mysql_query(mysql, sql_cmd);
        if (ret != 0)
        {
            std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
        }
        MYSQL_RES *result = mysql_store_result(mysql);
        if (result == NULL)
        {
            std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
            return;
        }

        int num_rows = mysql_num_rows(result);
        for (int i = 0; i < num_rows; i++)
        {
            MYSQL_ROW row = mysql_fetch_row(result);
            if (row == NULL)
            {
                std::cerr << "[ERR] mysql fetch row error: " << mysql_error(mysql) << std::endl;
                break;
            }
            if (atoi(row[0]) == findId()) // 自己不用发
                continue;
            if (checkUserOnline(atoi(row[0])))
            {
                send_data(msg_back, findCfd(atoi(row[0])));
            }

            // 管它再没在线，都要先存起来
            char sql_cmd1[256];
            snprintf(sql_cmd1, sizeof(sql_cmd1), "insert into filemessage values('%d', '%d', '%s', '%d','%s');", findId(), atoi(row[0]), msg_back.data.c_str(), 0, msg.filename.c_str());
            ret = mysql_query(mysql, sql_cmd1);
            if (ret != 0)
            {
                std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
            }
        }
    }
    if (msg.id != 0) // 表示是给好友发
    {
        msg_back.data = "给你发了一个文件" + msg.filename;
        if (checkUserOnline(msg.id))
        {
            send_data(msg_back, findCfd(msg.id));
        }

        // 管它再没在线，都要先存起来
        char sql_cmd2[256];
        snprintf(sql_cmd2, sizeof(sql_cmd2), "insert into filemessage values('%d', '%d', '%s', '%d','%s');", findId(), msg.id, msg_back.data.c_str(), 0, msg.filename.c_str());
        int ret = mysql_query(mysql, sql_cmd2);
        if (ret != 0)
        {
            std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
        }
    }
}
int Person::fileRestore()
{
    struct protocol msg_back;
    char sql_cmd[256];
    snprintf(sql_cmd, sizeof(sql_cmd), "select * from filemessage where (toid='%d' and status ='%d');", findId(), 0);
    int ret = mysql_query(mysql, sql_cmd);
    if (ret != 0)
    {
        std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    MYSQL_RES *result = mysql_store_result(mysql);
    if (result == NULL)
    {
        std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
        return -1; // Error indicator
    }
    int num_rows = mysql_num_rows(result);
    std::cout << "num_rows: " << num_rows << std::endl;
    for (int i = 0; i < num_rows; i++)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        if (row == NULL)
        {
            std::cerr << "[ERR] mysql fetch row error: " << mysql_error(mysql) << std::endl;
            break;
        }
        msg_back.id = atoi(row[0]);
        msg_back.data = row[2];
        msg_back.state = RECEIVEFILE_OK;
        send_data(msg_back, sockfd);
    }
    if (num_rows == 0) // 根本没有未读的文件或者文件已经读完了
    {
        msg_back.state = NOFILE;
        send_data(msg_back, sockfd);
        return 0;
    }
    return 1;
}
void Person::receiveFile()
{
    struct protocol msg_back;
    if (msg.state == RECEIVEFILE_OK)
    {
        // 先把发给他但是他还没接收的文件传给他
        if (fileRestore())
        {
            msg_back.state = RECEIVEFILE_END;
            send_data(msg_back, sockfd);
        }
    }
    if (msg.state == OP_OK)
    {
        // 先查找要收的文件有人给他发没
        char sql_cmd[256];
        snprintf(sql_cmd, sizeof(sql_cmd), "select * from filemessage where (filename='%s' and toid='%d' and status ='%d');", msg.filename.c_str(), findId(), 0);
        int ret = mysql_query(mysql, sql_cmd);
        if (ret != 0)
        {
            std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
        }
        MYSQL_RES *result = mysql_store_result(mysql);
        if (result == NULL)
        {
            std::cerr << "[ERR] mysql store result error: " << mysql_error(mysql) << std::endl;
            return;
        }

        int num_rows = mysql_num_rows(result);
        if (num_rows == 0) // 表示没有人给他发送这个文件
        {
            msg_back.state = NOFILE;
            send_data(msg_back, sockfd);
            return;
        }
        std::string filename = "/home/sweet/sever/" + msg.filename;
        // 开始发文件
        int file = open(filename.c_str(), O_RDONLY);
        if (file == -1)
        {
            std::cerr << "Failed to open file" << std::endl;
            return;
        }
        int original_flags = fcntl(sockfd, F_GETFL, 0);
        if (original_flags == -1)
        {
            std::cout << "Failed to get file descriptor flags" << std::endl;
            return;
        }
        if (fcntl(sockfd, F_SETFL, original_flags & ~O_NONBLOCK) == -1)
        {
            std::cout << "Failed to set file descriptor to blocking mode" << std::endl;
            return;
        }

        // 获取文件大小
        struct stat file_stat;
        fstat(file, &file_stat);
        msg_back.filesize = file_stat.st_size; // 获取文件大小
        msg_back.filename = msg.filename;
        msg_back.state = OP_OK;
        send_data(msg_back, sockfd);
        // 使用 sendfile 发送文件
        off_t offset = 0;
        ssize_t bytes_sent = 0;
        // 加这个是为了判断服务器是不是在发文件。在发就是1，不发就是0
        for (auto &map : mang)
        {
            // 遍历每个键值对
            for (auto &pair : map)
            {
                // 如果找到值为5的键
                if (pair.first == findId())
                {
                    // 将其对应的值改为3
                    pair.second = 1;
                }
            }
        }
        // 循环发送文件
        while (offset < file_stat.st_size)
        {
            bytes_sent = sendfile(sockfd, file, &offset, file_stat.st_size - offset);
            if (bytes_sent < 0)
            {
                std::cerr << "Sendfile error: " << strerror(errno) << "\n";
                break;
            }
            std::cout << "Sent " << bytes_sent << " bytes, total sent: " << offset << " bytes\n";
            std::cout << "\33[2K\r" << msg.filename << ": " << (int)(((float)offset / file_stat.st_size) * 100) << "%" << std::flush;
        }
        std::cout << std::endl;
        // 本来是想把收完放到这，但是发的快收的慢，发完不代表收完
        //  默认他发送成功，将状态由0变为1
        char sql_cmd3[256];
        snprintf(sql_cmd3, sizeof(sql_cmd3), "update filemessage set status = '%d' where (filename='%s' and toid='%d' and status ='%d');", 1, msg.filename.c_str(), findId(), 0);
        ret = mysql_query(mysql, sql_cmd3);
        if (ret != 0)
        {
            std::cerr << "[ERR] mysql select error: " << mysql_error(mysql) << std::endl;
        }

        // 关闭文件和 socket
        close(file);
    }
}
void Person::receiveOk()
{
    for (auto &map : mang)
    {
        for (auto &pair : map)
        {
            if (pair.first == findId())
            {
                pair.second = 0;
                std::cout << "hahahah" << std::endl;
            }
        }
    }
}