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
};

struct AuthResult {
    AuthErrorCode code = AuthErrorCode::Ok;
    std::string message = "OK";

    bool ok() const { return code == AuthErrorCode::Ok; }

    static AuthResult success(std::string message = "OK") {
        return {AuthErrorCode::Ok, std::move(message)};
    }

    static AuthResult fail(AuthErrorCode code, std::string message) {
        return {code, std::move(message)};
    }
};

inline std::string formatResponse(const AuthResult& result) {
    if (result.ok()) {
        return "OK\n";
    }
    return "ERR " + result.message + "\n";
}
