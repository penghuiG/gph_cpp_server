#pragma once

#include "accountReg.h"

class UserSignIn {
public:
    UserSignIn() = default;
    ~UserSignIn() = default;

    void signIn(const std::string& account, const std::string& password);
};