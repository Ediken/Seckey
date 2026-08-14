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

        // 查询最新一条可用密钥（校验/查看用）
    // 输出: outSeckey(密钥字符串) outKeyid(密钥ID)
    // 返回 0 找到 -1 没找到
    int getSecKey(const char* clientId, char* outSeckey, int* outKeyid);

    // 注销密钥：把 seckeyinfo 表里 keyid 对应记录置 state=1
    // 返回 0 成功 -1 失败
    int revokeSecKey(int keyid);

private:
    MYSQL* m_conn;    // 连接句柄
};

#endif // MYSQLOP_H
