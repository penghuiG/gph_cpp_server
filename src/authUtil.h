#pragma once

#include <string>

namespace auth {

inline std::string hashPassword(const std::string& password, const std::string& salt) {
    std::hash<std::string> h;
    return std::to_string(h(password + ":" + salt));
}

inline bool verifyStoredPassword(const std::string& password, const std::string& stored) {
    const auto pos = stored.find(':');
    if (pos == std::string::npos) {
        return false;
    }
    const std::string salt = stored.substr(0, pos);
    const std::string hash = stored.substr(pos + 1);
    return hashPassword(password, salt) == hash;
}

}  // namespace auth
