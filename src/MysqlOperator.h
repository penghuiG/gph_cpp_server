#pragma once

#include <string>
#include <vector>

#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>

class MysqlOperator {
public:
    MysqlOperator() = default;
    ~MysqlOperator();

    void connect(const std::string& host, const std::string& user, const std::string& password, const std::string& database);
    void disconnect();
    void execute(const std::string& query);
    void execute(const std::string& query, const std::vector<std::string>& params);

private:
    sql::mysql::MySQL_Driver* driver = nullptr;
    sql::Connection* connection = nullptr;
};