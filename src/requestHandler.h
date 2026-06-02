#pragma once

#include <string>

#include "accountReg.h"
#include "authResult.h"
#include "userSignIn.h"

class RequestHandler {
public:
    RequestHandler(AccountReg& accountReg, UserSignIn& userSignIn);

    std::string handle(const std::string& rawLine);

private:
    AccountReg& accountReg_;
    UserSignIn& userSignIn_;
};
