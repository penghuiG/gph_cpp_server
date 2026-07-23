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

#include <string>

#include "authResult.h"

class AccountReg {
public:
    AccountReg() = default;
    ~AccountReg() = default;

    AuthResult registerAccount(const std::string& account, const std::string& password);
    AuthResult unregisterAccount(const std::string& account);
    AuthResult changePassword(const std::string& account, const std::string& oldPassword, const std::string& newPassword);

private:
    AuthResult checkUsernameFormat(const std::string& username) const;
    AuthResult checkPasswordStrength(const std::string& password) const;
    bool checkUsernameExists(const std::string& username);

    std::string generateSalt();
    std::string hashPassword(const std::string& password, const std::string& salt);
};

void accountRegTest();
