#include "requestHandler.h"

#include "logger.h"

#include <sstream>

namespace {

std::string trim(std::string s) {
    while (!s.empty() && (s.back() == '\n' || s.back() == '\r' || s.back() == ' ' || s.back() == '\t')) {
        s.pop_back();
    }
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t')) {
        ++start;
    }
    return s.substr(start);
}

}  // namespace

RequestHandler::RequestHandler(AccountReg& accountReg, UserSignIn& userSignIn, UserProfile& userProfile)
    : accountReg_(accountReg), userSignIn_(userSignIn), userProfile_(userProfile) {}

std::string RequestHandler::handle(const std::string& rawLine) {
    try {
        const std::string line = trim(rawLine);
        if (line.empty()) {
            return formatResponse(AuthResult::fail(AuthErrorCode::BadRequest, "empty request"));
        }

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "REGISTER") {
            std::string username;
            std::string password;
            iss >> username >> password;
            if (username.empty() || password.empty()) {
                return formatResponse(
                    AuthResult::fail(AuthErrorCode::BadRequest, "usage: REGISTER username password"));
            }
            LOG_INFO << "cmd=REGISTER user=" << username;
            return formatResponse(accountReg_.registerAccount(username, password));
        }

        if (cmd == "UNREGISTER") {
            std::string username;
            iss >> username;
            if (username.empty()) {
                return formatResponse(
                    AuthResult::fail(AuthErrorCode::BadRequest, "usage: UNREGISTER username"));
            }
            LOG_INFO << "cmd=UNREGISTER user=" << username;
            return formatResponse(accountReg_.unregisterAccount(username));
        }

        if (cmd == "LOGIN") {
            std::string username;
            std::string password;
            iss >> username >> password;
            if (username.empty() || password.empty()) {
                return formatResponse(
                    AuthResult::fail(AuthErrorCode::BadRequest, "usage: LOGIN username password"));
            }
            LOG_INFO << "cmd=LOGIN user=" << username;
            const AuthResult result = userSignIn_.signIn(username, password);
            if (!result.ok()) {
                LOG_WARN << "login failed, user=" << username << ", reason=" << result.message;
            }
            return formatResponse(result);
        }

        if (cmd == "LOGOUT") {
            std::string username;
            iss >> username;
            if (username.empty()) {
                return formatResponse(
                    AuthResult::fail(AuthErrorCode::BadRequest, "usage: LOGOUT username"));
            }
            LOG_INFO << "cmd=LOGOUT user=" << username;
            return formatResponse(userSignIn_.signOut(username));
        }

        if (cmd == "CHECK") {
            std::string username;
            iss >> username;
            if (username.empty()) {
                return formatResponse(
                    AuthResult::fail(AuthErrorCode::BadRequest, "usage: CHECK username"));
            }
            LOG_DEBUG << "cmd=CHECK user=" << username;
            if (userSignIn_.checkSignIn(username)) {
                return formatResponse(AuthResult::success());
            }
            return formatResponse(AuthResult::fail(AuthErrorCode::InvalidCredentials, "not signed in"));
        }

        if (cmd == "GET_PROFILE") {
            std::string username;
            iss >> username;
            if (username.empty()) {
                return formatResponse(
                    AuthResult::fail(AuthErrorCode::BadRequest, "usage: GET_PROFILE username"));
            }
            LOG_INFO << "cmd=GET_PROFILE user=" << username;
            return formatResponse(userProfile_.getProfile(username));
        }

        if (cmd == "UPDATE_PROFILE") {
            std::string username;
            iss >> username;
            if (username.empty()) {
                return formatResponse(
                    AuthResult::fail(AuthErrorCode::BadRequest, "usage: UPDATE_PROFILE username [nickname=value] [email=value]"));
            }

            std::string nickname;
            std::string email;
            std::string token;
            while (iss >> token) {
                const auto eq = token.find('=');
                if (eq == std::string::npos) {
                    return formatResponse(
                        AuthResult::fail(AuthErrorCode::BadRequest, "invalid format, use key=value"));
                }
                const std::string key = token.substr(0, eq);
                const std::string value = token.substr(eq + 1);
                if (key == "nickname") {
                    nickname = value;
                } else if (key == "email") {
                    email = value;
                } else {
                    return formatResponse(
                        AuthResult::fail(AuthErrorCode::BadRequest, "unknown field: " + key + ", supported: nickname, email"));
                }
            }

            LOG_INFO << "cmd=UPDATE_PROFILE user=" << username;
            return formatResponse(userProfile_.updateProfile(username, nickname, email));
        }

        if (cmd == "CHANGE_PASSWORD") {
            std::string username;
            std::string oldPassword;
            std::string newPassword;
            iss >> username >> oldPassword >> newPassword;
            if (username.empty() || oldPassword.empty() || newPassword.empty()) {
                return formatResponse(
                    AuthResult::fail(AuthErrorCode::BadRequest, "usage: CHANGE_PASSWORD username old_password new_password"));
            }
            LOG_INFO << "cmd=CHANGE_PASSWORD user=" << username;
            return formatResponse(accountReg_.changePassword(username, oldPassword, newPassword));
        }

        if (cmd == "LIST_ONLINE") {
            LOG_DEBUG << "cmd=LIST_ONLINE";
            const auto users = userSignIn_.getOnlineUsers();
            if (users.empty()) {
                return formatResponse(AuthResult::successWithData("online users", "none"));
            }
            std::ostringstream oss;
            for (size_t i = 0; i < users.size(); ++i) {
                if (i > 0) oss << ",";
                oss << users[i];
            }
            return formatResponse(AuthResult::successWithData("online users", oss.str()));
        }

        return formatResponse(AuthResult::fail(AuthErrorCode::BadRequest, "unknown command"));
    } catch (const std::exception& e) {
        LOG_ERROR << "request handler exception: " << e.what();
        return formatResponse(AuthResult::fail(AuthErrorCode::DbError, e.what()));
    }
}