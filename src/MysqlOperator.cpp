#include "MysqlOperator.h"

#include <stdexcept>

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
    stmt->execute(query);
    delete stmt;
}

void MysqlOperator::execute(const std::string& query, const std::vector<std::string>& params) {
    if (!connection) throw std::runtime_error("MysqlOperator::execute called before connect()");
    sql::PreparedStatement* stmt = connection->prepareStatement(query);
    for (size_t i = 0; i < params.size(); i++) {
        stmt->setString(static_cast<unsigned int>(i + 1), params[i]);
    }
    stmt->execute();
    delete stmt;
}

MysqlOperator::~MysqlOperator() {
    try {
        disconnect();
    } catch (...) {
    }
}