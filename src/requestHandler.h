#pragma once

#include <string>

#include "accountReg.h"
#include "authResult.h"
#include "userProfile.h"
#include "userSignIn.h"

class RequestHandler {
public:
    RequestHandler(AccountReg& accountReg, UserSignIn& userSignIn, UserProfile& userProfile);

    std::string handle(const std::string& rawLine);

private:
    AccountReg& accountReg_;
    UserSignIn& userSignIn_;
    UserProfile& userProfile_;
};
