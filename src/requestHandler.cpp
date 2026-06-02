#include "requestHandler.h"

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

RequestHandler::RequestHandler(AccountReg& accountReg, UserSignIn& userSignIn)
    : accountReg_(accountReg), userSignIn_(userSignIn) {}

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
            return formatResponse(accountReg_.registerAccount(username, password));
        }

        if (cmd == "UNREGISTER") {
            std::string username;
            iss >> username;
            if (username.empty()) {
                return formatResponse(
                    AuthResult::fail(AuthErrorCode::BadRequest, "usage: UNREGISTER username"));
            }
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
            return formatResponse(userSignIn_.signIn(username, password));
        }

        if (cmd == "LOGOUT") {
            std::string username;
            iss >> username;
            if (username.empty()) {
                return formatResponse(
                    AuthResult::fail(AuthErrorCode::BadRequest, "usage: LOGOUT username"));
            }
            return formatResponse(userSignIn_.signOut(username));
        }

        if (cmd == "CHECK") {
            std::string username;
            iss >> username;
            if (username.empty()) {
                return formatResponse(
                    AuthResult::fail(AuthErrorCode::BadRequest, "usage: CHECK username"));
            }
            if (userSignIn_.checkSignIn(username)) {
                return formatResponse(AuthResult::success());
            }
            return formatResponse(AuthResult::fail(AuthErrorCode::InvalidCredentials, "not signed in"));
        }

        return formatResponse(AuthResult::fail(AuthErrorCode::BadRequest, "unknown command"));
    } catch (const std::exception& e) {
        return formatResponse(AuthResult::fail(AuthErrorCode::DbError, e.what()));
    }
}
