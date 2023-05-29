#pragma once
#include <jsoncpp/json/json.h>
#include <mysql/mysql.h>
#include <string>
#include <jwt-cpp/jwt.h>
#include "../lock/locker.h"
#include "../log/log.h"

class Logic
{
public:
    Logic(MYSQL* mysql, int close_log):mysql_(mysql), m_close_log(close_log){};
    ~Logic() = default;
    std::string getToken(int mg_id);
    bool checkToken(std::string token);
    void loginLogic(char* user_data, char* temp_buff, int &len);

private:
    MYSQL* mysql_;
    int m_close_log;
    
};