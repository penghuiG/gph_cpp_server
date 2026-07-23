#pragma once

#include <string>

enum class AuthErrorCode {
    Ok = 0,
    InvalidUsername,
    WeakPassword,
    UsernameExists,
    InvalidCredentials,
    BadRequest,
    DbError,
    UserNotFound,
};

struct AuthResult {
    AuthErrorCode code = AuthErrorCode::Ok;
    std::string message = "OK";
    std::string data;

    bool ok() const { return code == AuthErrorCode::Ok; }

    static AuthResult success(std::string message = "OK") {
        return {AuthErrorCode::Ok, std::move(message), {}};
    }

    static AuthResult successWithData(std::string message, std::string data) {
        return {AuthErrorCode::Ok, std::move(message), std::move(data)};
    }

    static AuthResult fail(AuthErrorCode code, std::string message) {
        return {code, std::move(message), {}};
    }
};

inline std::string formatResponse(const AuthResult& result) {
    if (result.ok()) {
        if (result.data.empty()) {
            return "OK\n";
        }
        return "OK " + result.data + "\n";
    }
    return "ERR " + result.message + "\n";
}
