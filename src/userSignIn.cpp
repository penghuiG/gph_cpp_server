#include "userSignIn.h"

void UserSignIn::signIn(const std::string& account, const std::string& password) {
    std::lock_guard<std::mutex> lk(mu);
    users.erase(account);
}
