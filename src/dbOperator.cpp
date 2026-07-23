#include "dbOperator.h"

#include <sstream>
#include <stdexcept>

#include "logger.h"

namespace {

void printResultSet(sql::ResultSet* res) {
    sql::ResultSetMetaData* meta = res->getMetaData();
    const int columnCount = meta->getColumnCount();
    while (res->next()) {
        std::ostringstream row;
        for (int i = 1; i <= columnCount; ++i) {
            if (i > 1) row << '\t';
            row << res->getString(i);
        }
        LOG_DEBUG << "query result: " << row.str();
    }
}

}

void MysqlOperator::connect(const std::string& host, const std::string& user, const std::string& password, const std::string& database) {
    driver = sql::mysql::get_mysql_driver_instance();
    connection = driver->connect(host, user, password);
    if (!database.empty()) {
        connection->setSchema(database);
    }
}

void MysqlOperator::disconnect() {
    if (!connection) return;
    connection->close();
    delete connection;
    connection = nullptr;
}

void MysqlOperator::execute(const std::string& query) {
    if (!connection) throw std::runtime_error("MysqlOperator::execute called before connect()");
    sql::Statement* stmt = connection->createStatement();
    sql::ResultSet* res = stmt->executeQuery(query);
    printResultSet(res);
    delete res;
    delete stmt;
}

void MysqlOperator::execute(const std::string& query, const std::vector<std::string>& params) {
    if (!connection) throw std::runtime_error("MysqlOperator::execute called before connect()");
    sql::PreparedStatement* stmt = connection->prepareStatement(query);
    for (size_t i = 0; i < params.size(); i++) {
        stmt->setString(static_cast<unsigned int>(i + 1), params[i]);
    }
    sql::ResultSet* res = stmt->executeQuery();
    printResultSet(res);
    delete res;
    delete stmt;
}

bool MysqlOperator::exists(const std::string& query, const std::vector<std::string>& params) {
    if (!connection) throw std::runtime_error("MysqlOperator::exists called before connect()");
    sql::PreparedStatement* stmt = connection->prepareStatement(query);//准备语句
    for (size_t i = 0; i < params.size(); i++) {
        stmt->setString(static_cast<unsigned int>(i + 1), params[i]);
    }
    sql::ResultSet* res = stmt->executeQuery();
    const bool found = res->next();//判断是否存在,如果存在则返回true,否则返回false
    delete res;
    delete stmt;
    return found;
}

bool MysqlOperator::queryOne(const std::string& query, const std::vector<std::string>& params, std::string& out) {
    if (!connection) throw std::runtime_error("MysqlOperator::queryOne called before connect()");
    sql::PreparedStatement* stmt = connection->prepareStatement(query);
    for (size_t i = 0; i < params.size(); i++) {
        stmt->setString(static_cast<unsigned int>(i + 1), params[i]);
    }
    sql::ResultSet* res = stmt->executeQuery();
    if (!res->next()) {
        delete res;
        delete stmt;
        return false;
    }
    out = res->getString(1);
    delete res;
    delete stmt;
    return true;
}

bool MysqlOperator::queryRow(const std::string& query, const std::vector<std::string>& params, std::vector<std::string>& out) {
    if (!connection) throw std::runtime_error("MysqlOperator::queryRow called before connect()");
    sql::PreparedStatement* stmt = connection->prepareStatement(query);
    for (size_t i = 0; i < params.size(); i++) {
        stmt->setString(static_cast<unsigned int>(i + 1), params[i]);
    }
    sql::ResultSet* res = stmt->executeQuery();
    if (!res->next()) {
        delete res;
        delete stmt;
        return false;
    }
    sql::ResultSetMetaData* meta = res->getMetaData();
    const int colCount = meta->getColumnCount();
    out.clear();
    out.reserve(static_cast<size_t>(colCount));
    for (int i = 1; i <= colCount; ++i) {
        out.push_back(res->getString(i));
    }
    delete res;
    delete stmt;
    return true;
}

std::vector<std::vector<std::string>> MysqlOperator::queryAll(const std::string& query, const std::vector<std::string>& params) {
    if (!connection) throw std::runtime_error("MysqlOperator::queryAll called before connect()");
    std::vector<std::vector<std::string>> rows;
    sql::PreparedStatement* stmt = connection->prepareStatement(query);
    for (size_t i = 0; i < params.size(); i++) {
        stmt->setString(static_cast<unsigned int>(i + 1), params[i]);
    }
    sql::ResultSet* res = stmt->executeQuery();
    sql::ResultSetMetaData* meta = res->getMetaData();
    const int colCount = meta->getColumnCount();
    while (res->next()) {
        std::vector<std::string> row;
        row.reserve(static_cast<size_t>(colCount));
        for (int i = 1; i <= colCount; ++i) {
            row.push_back(res->getString(i));
        }
        rows.push_back(std::move(row));
    }
    delete res;
    delete stmt;
    return rows;
}

void MysqlOperator::executeUpdate(const std::string& query, const std::vector<std::string>& params) {
    if (!connection) throw std::runtime_error("MysqlOperator::executeUpdate called before connect()");
    if (params.empty()) {
        sql::Statement* stmt = connection->createStatement();
        stmt->executeUpdate(query);
        delete stmt;
        return;
    }
    sql::PreparedStatement* stmt = connection->prepareStatement(query);
    for (size_t i = 0; i < params.size(); i++) {
        stmt->setString(static_cast<unsigned int>(i + 1), params[i]);
    }
    stmt->executeUpdate();
    delete stmt;
}

MysqlOperator::~MysqlOperator() {
    try {
        disconnect();
    } catch (...) {
    }
}
/*
INSERT INTO users (username, password_hash, nickname)
VALUES ('testuser', '$2b$12$xxxxxxxxxxxxxxxxxxxxxx', '测试用户');
*/

int mysql_operator_test() {
    MysqlOperator mysql_operator;
    mysql_operator.connect("127.0.0.1:3306", "auth_user", "123456", "auth_db");
    mysql_operator.execute("SELECT * FROM users");
    return 0;
}