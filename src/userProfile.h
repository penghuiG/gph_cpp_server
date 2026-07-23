#pragma once

#include <string>

#include "authResult.h"

class UserProfile {
public:
    UserProfile() = default;
    ~UserProfile() = default;

    // GET_PROFILE username - returns nickname, email, created_at
    AuthResult getProfile(const std::string& username);

    // UPDATE_PROFILE username [nickname=value] [email=value]
    AuthResult updateProfile(const std::string& username,
                             const std::string& nickname,
                             const std::string& email);
};