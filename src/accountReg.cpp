#include "accountReg.h"

#include <random>
#include <regex>
#include <stdexcept>

#include "MysqlOperator.h"
#include "authUtil.h"
#include "dbConfig.h"

void AccountReg::registerAccount(const std::string& account, const std::string& password) {
    checkUsernameFormat(account);
    checkPasswordStrength(password);
    checkUsernameExists(account);

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
}

void AccountReg::unregisterAccount(const std::string& account) {
    MysqlOperator mysql_operator;
    mysql_operator.connect(db::kHost, db::kUser, db::kPassword, db::kName);
    mysql_operator.executeUpdate(
        "UPDATE users SET is_active = 0 WHERE username = ? AND is_active = 1",
        {account});
    mysql_operator.disconnect();
}

void AccountReg::checkUsernameFormat(const std::string& username) {
    static const std::regex pattern("^[a-zA-Z0-9_]{3,50}$");
    if (!std::regex_match(username, pattern)) {
        throw std::runtime_error("Invalid username format");
    }
}

void AccountReg::checkPasswordStrength(const std::string& password) {
    static const std::regex pattern("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d).{8,}$");
    if (!std::regex_match(password, pattern)) {
        throw std::runtime_error("Password does not meet strength requirements");
    }
}

void AccountReg::checkUsernameExists(const std::string& username) {
    MysqlOperator mysql_operator;
    mysql_operator.connect(db::kHost, db::kUser, db::kPassword, db::kName);
    if (mysql_operator.exists(
            "SELECT 1 FROM users WHERE username = ? AND is_active = 1 LIMIT 1",
            {username})) {
        throw std::runtime_error("Username already exists");
    }
    mysql_operator.disconnect();
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

void accountRegTest() {
    AccountReg accountReg;
    accountReg.unregisterAccount("xhh");
    accountReg.registerAccount("xhh", "Xhh123456");
    accountReg.unregisterAccount("xhh");
}
