-- ============================================================
-- seckeydb.sql —— 密钥协商项目数据库脚本（MySQL 版）
-- 用法: mysql -uroot -p < server/seckeydb.sql
-- ============================================================

-- 建库（utf8mb4 支持中文）
CREATE DATABASE IF NOT EXISTS seckeydb DEFAULT CHARACTER SET utf8mb4;
USE seckeydb;

-- 1. 网点表：合法客户端/服务端（替代代码里的白名单校验）
DROP TABLE IF EXISTS secnode;
CREATE TABLE secnode (
    id         INT NOT NULL AUTO_INCREMENT,   -- 网点ID
    name       VARCHAR(64) NOT NULL,          -- 网点名称
    nodedesc   VARCHAR(256),                  -- 描述
    createtime DATETIME DEFAULT CURRENT_TIMESTAMP,  -- 创建时间
    authcode   VARCHAR(64),                   -- 认证码
    state      INT DEFAULT 0,                 -- 0=正常 1=注销
    PRIMARY KEY (id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 插入测试网点（对应我们代码 clientID="1111" serverID="0001"）
INSERT INTO secnode (id, name, nodedesc) VALUES
(1111, '深圳分中心', '测试客户端'),
(1,    '网银中心',   '测试服务端');

-- 2. 密钥表：协商出的密钥（服务端写入，AUTO_INCREMENT 替代 keysn 序列）
DROP TABLE IF EXISTS seckeyinfo;
CREATE TABLE seckeyinfo (
    keyid      INT NOT NULL AUTO_INCREMENT,   -- 密钥ID（自增）
    clientid   VARCHAR(12) NOT NULL,          -- 客户端ID（字符串）
    serverid   VARCHAR(12) NOT NULL,          -- 服务端ID
    seckey     VARCHAR(128) NOT NULL,         -- 密钥（SHA1 40字符hex）
    createtime DATETIME DEFAULT CURRENT_TIMESTAMP,
    state      INT DEFAULT 0,                 -- 0=可用 1=注销
    PRIMARY KEY (keyid)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;

-- 3. 展示建表结果
SHOW TABLES;
