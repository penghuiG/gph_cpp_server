#pragma once

#include <mutex>
#include <string>
#include <unordered_set>

#include "authResult.h"

class UserSignIn {
public:
    UserSignIn() = default;
    ~UserSignIn() = default;

    AuthResult signIn(const std::string& account, const std::string& password);
    AuthResult signOut(const std::string& account);
    bool checkSignIn(const std::string& account);

private:
    std::unordered_set<std::string> sessions_;
    std::mutex mu_;
};

void userSignInTest();
