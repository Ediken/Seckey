#ifndef MYSQLOP_H
#define MYSQLOP_H

#include <mysql.h>     // MySQL C API
#include <string.h>
// ============================================================
class MysqlOP
{
public:
    MysqlOP();
    ~MysqlOP();

    // 连接数据库
    // 返回 0 成功 -1 失败
    int connectDB(const char* host, const char* user,
                  const char* passwd, const char* dbname);

    // 校验客户端是否合法：查 secnode 表（按 id 和 state）
    // 返回 0 合法 -1 不合法（或查询失败）
    int checkClient(const char* clientId, const char* serverId);

    // 写入密钥：插进 seckeyinfo 表
    // 返回 0 成功 -1 失败
    int writeSecKey(const char* clientId, const char* serverId, const char* seckey);

private:
    MYSQL* m_conn;    // 连接句柄
};

#endif // MYSQLOP_H
