#pragma once
/*
CREATE TABLE users (
    id BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    password_hash CHAR(60) NOT NULL,
    nickname VARCHAR(50),
    email VARCHAR(100) UNIQUE,
    is_active TINYINT(1) NOT NULL DEFAULT 1,
    created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP
);
*/

/*
客户端请求注册
↓
服务器校验参数
  ├─ 用户名格式
  ├─ 密码强度
  └─ 用户名是否存在
↓
生成 salt
↓
hash = Hash(password + salt)
↓
写入数据库
↓
返回注册结果
*/
#include <string>
#include <mutex>
#include <unordered_map>

class AccountReg {
public:
    AccountReg() = default;
    ~AccountReg() = default;

    // 注册账号；失败抛异常（用户名格式/密码强度/重复用户名等）
    void registerAccount(const std::string& account, const std::string& password);
    //注销账号
    void unregisterAccount(const std::string& account);
private:
    void checkUsernameFormat(const std::string& username);
    void checkPasswordStrength(const std::string& password);
    void checkUsernameExists(const std::string& username);

private:
    // 先做一个最小可编译实现：用内存表模拟用户库
    // 后续如果要接 MySQL，再把实现替换掉即可
    std::string generateSalt();
    std::string hashPassword(const std::string& password, const std::string& salt);

private:
    // username -> "salt:hash"
    std::unordered_map<std::string, std::string> users;
    std::mutex mu;
};

void accountRegTest();