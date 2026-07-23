#include "userProfile.h"

#include <sstream>

#include "dbConfig.h"
#include "dbOperator.h"
#include "logger.h"

AuthResult UserProfile::getProfile(const std::string& username) {
    MysqlOperator mysql_operator;
    mysql_operator.connect(db::kHost, db::kUser, db::kPassword, db::kName);

    std::vector<std::string> row;
    const bool found = mysql_operator.queryRow(
        "SELECT username, nickname, email, created_at FROM users WHERE username = ? AND is_active = 1 LIMIT 1",
        {username},
        row);
    mysql_operator.disconnect();

    if (!found) {
        return AuthResult::fail(AuthErrorCode::UserNotFound, "user not found");
    }

    // row: [username, nickname, email, created_at]
    std::ostringstream oss;
    oss << "username=" << row[0]
        << " nickname=" << row[1]
        << " email=" << row[2]
        << " created_at=" << row[3];

    return AuthResult::successWithData("profile", oss.str());
}

AuthResult UserProfile::updateProfile(const std::string& username,
                                       const std::string& nickname,
                                       const std::string& email) {
    MysqlOperator mysql_operator;
    mysql_operator.connect(db::kHost, db::kUser, db::kPassword, db::kName);

    // Check user exists
    if (!mysql_operator.exists(
            "SELECT 1 FROM users WHERE username = ? AND is_active = 1 LIMIT 1",
            {username})) {
        mysql_operator.disconnect();
        return AuthResult::fail(AuthErrorCode::UserNotFound, "user not found");
    }

    if (!nickname.empty() && !email.empty()) {
        mysql_operator.executeUpdate(
            "UPDATE users SET nickname = ?, email = ? WHERE username = ?",
            {nickname, email, username});
    } else if (!nickname.empty()) {
        mysql_operator.executeUpdate(
            "UPDATE users SET nickname = ? WHERE username = ?",
            {nickname, username});
    } else if (!email.empty()) {
        mysql_operator.executeUpdate(
            "UPDATE users SET email = ? WHERE username = ?",
            {email, username});
    } else {
        mysql_operator.disconnect();
        return AuthResult::fail(AuthErrorCode::BadRequest, "nothing to update, provide nickname= or email=");
    }

    mysql_operator.disconnect();
    return AuthResult::success("profile updated");
}