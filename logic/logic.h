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
    // todo debug len可能会出问题
    Logic(MYSQL *mysql, int close_log, char *temp_buff, int *len) : mysql_(mysql),
                                                                    m_close_log(close_log),
                                                                    temp_buff_(temp_buff),
                                                                    len_(len) {}

    Logic(MYSQL *mysql, int close_log, char *temp_buff, int *len, std::string token) : mysql_(mysql),
                                                                                       m_close_log(close_log),
                                                                                       temp_buff_(temp_buff),
                                                                                       len_(len)
    {
        LOG_DEBUG("get_token=>%s", token.c_str());
        is_token_vaild_ = checkToken(token, user_id_);
        // is_token_vaild_ = true;
        // user_id_ = 500;

        key_vector_ = std::make_shared<std::vector<std::string>>();
    }
    ~Logic() = default;
    std::string getToken(int mg_id);
    void loginLogic(char *user_data);
    void menuLogic();

private:
    MYSQL *mysql_;
    int m_close_log;
    int user_id_;
    bool is_token_vaild_;
    char *temp_buff_;
    int *len_;

    bool checkToken(std::string token, int &mg_id);
    std::shared_ptr<std::vector<std::string>> key_vector_;
    void getTableKey(string table_name);
    void clearTableKey();
    void errorLogic(int status, std::string msg);
    void cpyJson2Buff(Json::Value* ret_root);
    int indexOf(string key_name);
};