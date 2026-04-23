#include "userSignIn.h"

void UserSignIn::signIn(const std::string& account, const std::string& password) {
    std::lock_guard<std::mutex> lk(mu);
    users.emplace(account, password);
}

void UserSignIn::signOut(const std::string& account) {
    std::lock_guard<std::mutex> lk(mu);
    users.erase(account);
}

bool UserSignIn::checkSignIn(const std::string& account) {
    std::lock_guard<std::mutex> lk(mu);
    return users.find(account) != users.end();
}