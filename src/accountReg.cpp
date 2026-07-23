#include "accountReg.h"

#include <random>
#include <regex>

#include "authUtil.h"
#include "dbConfig.h"
#include "dbOperator.h"
#include "logger.h"

AuthResult AccountReg::registerAccount(const std::string& account, const std::string& password) {
    if (auto result = checkUsernameFormat(account); !result.ok()) {
        return result;
    }
    if (auto result = checkPasswordStrength(password); !result.ok()) {
        return result;
    }
    if (checkUsernameExists(account)) {
        return AuthResult::fail(AuthErrorCode::UsernameExists, "Username already exists");
    }

    MysqlOperator mysql_operator;
    mysql_operator.connect(db::kHost, db::kUser, db::kPassword, db::kName);
    const std::string salt = generateSalt();
    const std::string stored = salt + ":" + auth::hashPassword(password, salt);
    if (mysql_operator.exists("SELECT 1 FROM users WHERE username = ? LIMIT 1", {account})) {
        mysql_operator.executeUpdate(
            "UPDATE users SET password_hash = ?, nickname = ?, is_active = 1 WHERE username = ?",
            {stored, account, account});
    } else {
        mysql_operator.executeUpdate(
            "INSERT INTO users (username, password_hash, nickname) VALUES (?, ?, ?)",
            {account, stored, account});
    }
    mysql_operator.disconnect();
    return AuthResult::success();
    
}

AuthResult AccountReg::unregisterAccount(const std::string& account) {
    if (auto result = checkUsernameFormat(account); !result.ok()) {
        return result;
    }

    MysqlOperator mysql_operator;
    mysql_operator.connect(db::kHost, db::kUser, db::kPassword, db::kName);
    mysql_operator.executeUpdate(
        "UPDATE users SET is_active = 0 WHERE username = ? AND is_active = 1",
        {account});
    mysql_operator.disconnect();
    return AuthResult::success();
}

AuthResult AccountReg::checkUsernameFormat(const std::string& username) const {
    static const std::regex pattern("^[a-zA-Z0-9_]{3,50}$");
    if (!std::regex_match(username, pattern)) {
        return AuthResult::fail(AuthErrorCode::InvalidUsername, "Invalid username format");
    }
    return AuthResult::success();
}

AuthResult AccountReg::checkPasswordStrength(const std::string& password) const {
    static const std::regex pattern("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d).{8,}$");
    if (!std::regex_match(password, pattern)) {
        return AuthResult::fail(AuthErrorCode::WeakPassword, "Password does not meet strength requirements");
    }
    return AuthResult::success();
}

bool AccountReg::checkUsernameExists(const std::string& username) {
    MysqlOperator mysql_operator;
    mysql_operator.connect(db::kHost, db::kUser, db::kPassword, db::kName);
    const bool exists = mysql_operator.exists(
        "SELECT 1 FROM users WHERE username = ? AND is_active = 1 LIMIT 1",
        {username});
    mysql_operator.disconnect();
    return exists;
}

std::string AccountReg::generateSalt() {
    static const char charset[] =
        "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dist(0, static_cast<int>(sizeof(charset) - 2));

    std::string salt(32, '\0');
    for (char& c : salt) {
        c = charset[dist(gen)];
    }
    return salt;
}

std::string AccountReg::hashPassword(const std::string& password, const std::string& salt) {
    return auth::hashPassword(password, salt);
}

AuthResult AccountReg::changePassword(const std::string& account, const std::string& oldPassword, const std::string& newPassword) {
    if (auto result = checkUsernameFormat(account); !result.ok()) {
        return result;
    }
    if (auto result = checkPasswordStrength(newPassword); !result.ok()) {
        return result;
    }

    MysqlOperator mysql_operator;
    mysql_operator.connect(db::kHost, db::kUser, db::kPassword, db::kName);

    std::string stored;
    const bool found = mysql_operator.queryOne(
        "SELECT password_hash FROM users WHERE username = ? AND is_active = 1 LIMIT 1",
        {account},
        stored);
    if (!found) {
        mysql_operator.disconnect();
        return AuthResult::fail(AuthErrorCode::UserNotFound, "user not found");
    }
    if (!auth::verifyStoredPassword(oldPassword, stored)) {
        mysql_operator.disconnect();
        return AuthResult::fail(AuthErrorCode::InvalidCredentials, "old password is incorrect");
    }

    const std::string salt = generateSalt();
    const std::string newStored = salt + ":" + auth::hashPassword(newPassword, salt);
    mysql_operator.executeUpdate(
        "UPDATE users SET password_hash = ? WHERE username = ?",
        {newStored, account});
    mysql_operator.disconnect();
    return AuthResult::success("password changed");
}

void accountRegTest() {
    AccountReg accountReg;

    accountReg.unregisterAccount("xhh");
    if (auto result = accountReg.registerAccount("xhh", "Xhh123456"); !result.ok()) {
        LOG_ERROR << "register failed: " << result.message;
        return;
    }
    LOG_INFO << "register success";

    const AuthResult duplicate = accountReg.registerAccount("xhh", "Xhh123456");
    if (duplicate.ok()) {
        LOG_ERROR << "duplicate register should fail";
        return;
    }
    LOG_INFO << "duplicate register rejected: " << duplicate.message;
}
