#include "userSignIn.h"

#include <iostream>
#include <stdexcept>

#include "MysqlOperator.h"
#include "authUtil.h"
#include "dbConfig.h"

void UserSignIn::signIn(const std::string& account, const std::string& password) {
    MysqlOperator mysql_operator;
    mysql_operator.connect(db::kHost, db::kUser, db::kPassword, db::kName);

    std::string stored;
    const bool found = mysql_operator.queryOne(
        "SELECT password_hash FROM users WHERE username = ? AND is_active = 1 LIMIT 1",
        {account},
        stored);
    mysql_operator.disconnect();

    if (!found || !auth::verifyStoredPassword(password, stored)) {
        throw std::runtime_error("Invalid username or password");
    }

    std::lock_guard<std::mutex> lk(mu_);
    sessions_.insert(account);
}

void UserSignIn::signOut(const std::string& account) {
    std::lock_guard<std::mutex> lk(mu_);
    sessions_.erase(account);
}

bool UserSignIn::checkSignIn(const std::string& account) {
    std::lock_guard<std::mutex> lk(mu_);
    return sessions_.count(account) > 0;
}

void userSignInTest() {
    AccountReg accountReg;
    accountReg.unregisterAccount("xhh");
    accountReg.registerAccount("xhh", "Xhh123456");

    UserSignIn userSignIn;
    userSignIn.signIn("xhh", "Xhh123456");
    if (userSignIn.checkSignIn("xhh")) {
        std::cout << "sign in success" << std::endl;
    } else {
        std::cout << "sign in failed" << std::endl;
    }

    userSignIn.signOut("xhh");
    if (!userSignIn.checkSignIn("xhh")) {
        std::cout << "sign out success" << std::endl;
    }

    accountReg.unregisterAccount("xhh");
}
