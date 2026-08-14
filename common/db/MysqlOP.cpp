#include "MysqlOP.h"
#include <stdio.h>

MysqlOP::MysqlOP()
{
    m_conn = NULL;
}

MysqlOP::~MysqlOP()
{
    if (m_conn != NULL)
    {
        mysql_close(m_conn);    // 关闭连接
        m_conn = NULL;
    }
}

int MysqlOP::connectDB(const char* host, const char* user,
                       const char* passwd, const char* dbname)
{
    // 1. 初始化连接句柄
    m_conn = mysql_init(NULL);
    if (m_conn == NULL) return -1;

    // 2. 建立连接（3306 是 MySQL 默认端口）
    if (mysql_real_connect(m_conn, host, user, passwd, dbname, 3306, NULL, 0) == NULL)
    {
        fprintf(stderr, "mysql_real_connect error: %s\n", mysql_error(m_conn));
        return -1;
    }

    // 3. 设置字符集（防中文乱码）
    mysql_set_character_set(m_conn, "utf8mb4");
    return 0;
}

int MysqlOP::checkClient(const char* clientId, const char* serverId)
{
    if (m_conn == NULL || clientId == NULL || serverId == NULL) return -1;

    // 拼 SQL：查这个 clientId 对应的网点是否存在且正常(state=0)
    char sql[256];
    sprintf(sql, "select id from secnode where id=%s and state=0", clientId);

    if (mysql_query(m_conn, sql) != 0)
    {
        fprintf(stderr, "query error: %s\n", mysql_error(m_conn));
        return -1;
    }

    // 取结果集
    MYSQL_RES* res = mysql_store_result(m_conn);
    if (res == NULL) return -1;

    // 查到了 → 合法
    int ret = (mysql_num_rows(res) > 0) ? 0 : -1;
    mysql_free_result(res);
    return ret;
}

// 写入密钥：insert into seckeyinfo(clientid, serverid, seckey) values(...)
int MysqlOP::writeSecKey(const char* clientId, const char* serverId, const char* seckey)
{
    if (m_conn == NULL || clientId == NULL || serverId == NULL || seckey == NULL)
        return -1;

    char sql[512];
    sprintf(sql, "insert into seckeyinfo(clientid, serverid, seckey, state) "
                 "values('%s', '%s', '%s', 0)",
            clientId, serverId, seckey);

    if (mysql_query(m_conn, sql) != 0)
    {
        fprintf(stderr, "insert error: %s\n", mysql_error(m_conn));
        return -1;
    }
    return 0;
}
