#pragma once

#include "accountReg.h"

#include <mutex>
#include <string>
#include <unordered_set>

class UserSignIn {
public:
    UserSignIn() = default;
    ~UserSignIn() = default;

    void signIn(const std::string& account, const std::string& password);
    void signOut(const std::string& account);
    bool checkSignIn(const std::string& account);

private:
    std::unordered_set<std::string> sessions_;
    std::mutex mu_;
};

void userSignInTest();
