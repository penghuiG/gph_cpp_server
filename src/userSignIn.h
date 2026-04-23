#pragma once

#include "accountReg.h"
#include <unordered_map>
#include <mutex>


class UserSignIn {
public:
    UserSignIn() = default;
    ~UserSignIn() = default;

    void signIn(const std::string& account, const std::string& password);
    void signOut(const std::string& account);
    bool checkSignIn(const std::string& account);

private:
    std::unordered_map<std::string, std::string> users;
    std::mutex mu;
};