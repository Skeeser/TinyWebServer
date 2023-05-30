#pragma once
#include <jsoncpp/json/json.h>
#include <mysql/mysql.h>
#include <string>
#include <jwt-cpp/jwt.h>
#include <vector>
#include <map>
#include <memory>
#include "../lock/locker.h"
#include "../log/log.h"

class Logic
{
public:
    Logic(MYSQL *mysql, int close_log) : mysql_(mysql), m_close_log(close_log) {}

    Logic(MYSQL *mysql, int close_log, std::string token) : mysql_(mysql), m_close_log(close_log)
    {
        LOG_DEBUG("get_token=>%s", token.c_str());
        is_token_vaild_ = checkToken(token, user_id_);
        // is_token_vaild_ = true;
        // user_id_ = 500;

        key_vector_ = std::make_shared<std::vector<std::string>>();
    }
    ~Logic() = default;
    std::string getToken(int mg_id);
    bool checkToken(std::string token, int &mg_id);

    void loginLogic(char *user_data, char *temp_buff, int &len);
    void menuLogic(char *temp_buff, int &len);

private:
    MYSQL *mysql_;
    int m_close_log;
    int user_id_;
    bool is_token_vaild_;
    std::shared_ptr<std::vector<std::string>> key_vector_;
    void getTableKey(std::vector<std::string> *key_vector, string table_name);
    void tokenUnvaildLogic(char *temp_buff, int &len);
    int indexOf(string key_name);
};