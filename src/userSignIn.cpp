#include "userSignIn.h"

#include <iostream>

#include "authUtil.h"
#include "dbConfig.h"
#include "dbOperator.h"
#include "logger.h"

AuthResult UserSignIn::signIn(const std::string& account, const std::string& password) {
    MysqlOperator mysql_operator;
    mysql_operator.connect(db::kHost, db::kUser, db::kPassword, db::kName);

    std::string stored;
    const bool found = mysql_operator.queryOne(
        "SELECT password_hash FROM users WHERE username = ? AND is_active = 1 LIMIT 1",
        {account},
        stored);
    mysql_operator.disconnect();

    if (!found || !auth::verifyStoredPassword(password, stored)) {
        return AuthResult::fail(AuthErrorCode::InvalidCredentials, "Invalid username or password");
    }

    std::lock_guard<std::mutex> lk(mu_);
    sessions_.insert(account);
    return AuthResult::success();
}

AuthResult UserSignIn::signOut(const std::string& account) {
    std::lock_guard<std::mutex> lk(mu_);
    sessions_.erase(account);
    return AuthResult::success();
}

bool UserSignIn::checkSignIn(const std::string& account) {
    std::lock_guard<std::mutex> lk(mu_);
    return sessions_.count(account) > 0;
}

void userSignInTest() {
    UserSignIn userSignIn;

    if (auto result = userSignIn.signIn("xhh", "Xhh123456"); !result.ok()) {
        LOG_ERROR << "sign in failed: " << result.message;
        return;
    }
    if (userSignIn.checkSignIn("xhh")) {
        LOG_INFO << "sign in success";
    }

    userSignIn.signOut("xhh");
    if (!userSignIn.checkSignIn("xhh")) {
        LOG_INFO << "sign out success";
    }
}
