#include "accountReg.h"

#include <random>
#include <regex>
#include <stdexcept>

void AccountReg::registerAccount(const std::string& account, const std::string& password) {
    checkUsernameFormat(account);
    checkPasswordStrength(password);

    std::lock_guard<std::mutex> lk(mu);
    checkUsernameExists(account);

    const std::string salt = generateSalt();
    const std::string hash = hashPassword(password, salt);
    users.emplace(account, salt + ":" + hash);
}

void AccountReg::unregisterAccount(const std::string& account) {
    std::lock_guard<std::mutex> lk(mu);
    users.erase(account);
}

void AccountReg::checkUsernameFormat(const std::string& username) {
    if (username.length() < 3 || username.length() > 16) {
        throw std::runtime_error("Username must be between 3 and 16 characters");   
    }
    if (!std::regex_match(username, std::regex("^[a-zA-Z0-9]+$"))) {
        throw std::runtime_error("Username must contain only letters and numbers");
    }
}

void AccountReg::checkPasswordStrength(const std::string& password) {
    if (password.length() < 8) {
        throw std::runtime_error("Password must be at least 8 characters long");
    }
    if (!std::regex_match(password, std::regex("^(?=.*[a-z])(?=.*[A-Z])(?=.*\\d).+$"))) {
        throw std::runtime_error("Password must contain at least one lowercase letter, one uppercase letter, and one number");
    }
}

void AccountReg::checkUsernameExists(const std::string& username) {
    if (users.find(username) != users.end()) {
        throw std::runtime_error("Username already exists");
    }
}

std::string AccountReg::generateSalt() {//
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dist;
    uint64_t v = dist(gen);
    return std::to_string(v);
}

std::string AccountReg::hashPassword(const std::string& password, const std::string& salt) {
    // 仅用于 demo/占位：不是密码学安全哈希
    std::hash<std::string> h;
    return std::to_string(h(password + ":" + salt));
}

void accountRegTest() {
    AccountReg accountReg;
    accountReg.registerAccount("test", "aA123456");
}
