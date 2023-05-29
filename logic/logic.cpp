#include "logic.h"

#define WRITE_BUFFER_SIZE 4096
#define SECRET_KEY "3MnJb57tW9TAvkYFQEDUgLdSRuBzmXcZ"

std::string Logic::getToken(int mg_id)
{
    return jwt::create()
        .set_issuer("auth")
        .set_type("JWS")
        .set_payload_claim("mg_id", jwt::claim(std::to_string(mg_id)))
        .sign(jwt::algorithm::hs256{SECRET_KEY});
}

bool Logic::checkToken(std::string token)
{
    auto decoded = jwt::decode(token);
    auto verifier = jwt::verify()
                        .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
                        .with_issuer("auth");
    try
    {
        verifier.verify(decoded);

        return true;
    }
    catch (const std::exception &e)
    {
        return false;
    }
}

void Logic::loginLogic(char *user_data, char *temp_buff, int &len)
{

    // 创建 JSON 对象
    Json::Value root;

    Json::Reader reader;
    Json::StreamWriterBuilder writer;
    std::string json_string(user_data);
    if (!reader.parse(json_string, root))
    {
        LOG_INFO("sorry, json reader failed");
    }

    std::string sql_string("SELECT * FROM sp_manager WHERE mg_name = '");
    sql_string += root["username"].asString();
    sql_string += "' AND mg_pwd = '";
    sql_string += root["password"].asString() + "';";

    Json::Value ret_root;
    Json::Value data;
    Json::Value meta;
    int mg_id = -1;
    // m_lock.lock();
    if (mysql_ == NULL)
        LOG_INFO("mysql is NULL!");

    LOG_INFO("sql_string=>%s", sql_string.c_str());
    int ret = mysql_query(mysql_, sql_string.c_str());
    // m_lock.unlock();
    // LOG_DEBUG("ret=>%d", ret);
    if (!ret) // 查询成功
    {
        // 从表中检索完整的结果集
        MYSQL_RES *result = mysql_store_result(mysql_);

        while (MYSQL_ROW row = mysql_fetch_row(result))
        {

            mg_id = std::stoi(row[0]);
            LOG_INFO("row=>%d", mg_id);
        }

        data["username"] = root["username"].asString();
        data["token"] = getToken(mg_id);
        meta["msg"] = "登录成功";
        meta["status"] = 200;
        ret_root["data"] = data;
        ret_root["meta"] = meta;
    }
    else
    {
        data["username"] = root["username"].asString();
        meta["msg"] = "登录失败";
        meta["status"] = 404;
        ret_root["data"] = data;
        ret_root["meta"] = meta;
    }

    // 清空
    memset(temp_buff, '\0', WRITE_BUFFER_SIZE);

    // 将 JSON 对象转换为字符串
    std::string jsonString = Json::writeString(writer, ret_root);

    len = jsonString.size();
    // LOG_DEBUG("json_string = %s, len = %d", jsonString.c_str(), len);
    if (len <= WRITE_BUFFER_SIZE)
    {
        strncpy(temp_buff, jsonString.c_str(), len);
        // LOG_DEBUG("ret_json=>%s", temp_buff);
    }
}

void Logic::menuLogic(char *temp_buff, int &len)
{

    std::string sql_string("SELECT * FROM sp_permission_api as api LEFT JOIN sp_permission as main ON main.ps_id = api.ps_id WHERE main.ps_id is not null;");

    Json::Value ret_root;
    Json::Value data;
    Json::Value meta;
    int mg_id = -1;
    // m_lock.lock();
    if (mysql_ == NULL)
        LOG_INFO("mysql is NULL!");

    int ret = mysql_query(mysql_, sql_string.c_str());
    if (!ret) // 查询成功
    {
        // 从表中检索完整的结果集
        MYSQL_RES *result = mysql_store_result(mysql_);

        while (MYSQL_ROW row = mysql_fetch_row(result))
        {

            mg_id = std::stoi(row[0]);
            // LOG_INFO("row=>%d", mg_id);
        }

        data["username"] = root["username"].asString();
        data["token"] = getToken(mg_id);
        meta["msg"] = "登录成功";
        meta["status"] = 200;
        ret_root["data"] = data;
        ret_root["meta"] = meta;
    }
    else
    {
        data["username"] = root["username"].asString();
        meta["msg"] = "登录失败";
        meta["status"] = 404;
        ret_root["data"] = data;
        ret_root["meta"] = meta;
    }

    // 清空
    memset(temp_buff, '\0', WRITE_BUFFER_SIZE);

    // 将 JSON 对象转换为字符串
    std::string jsonString = Json::writeString(writer, ret_root);

    len = jsonString.size();
    // LOG_DEBUG("json_string = %s, len = %d", jsonString.c_str(), len);
    if (len <= WRITE_BUFFER_SIZE)
    {
        strncpy(temp_buff, jsonString.c_str(), len);
        // LOG_DEBUG("ret_json=>%s", temp_buff);
    }
}

void Logic::getTableKey(std::vector<std::string> *key_vector, string table_name)
{

    std::string sql_string("SHOW COLUMNS from ");
    sql_string += table_name + " ;";

    if (mysql_ == NULL)
        LOG_INFO("mysql is NULL!");

    int ret = mysql_query(mysql_, sql_string.c_str());

    // LOG_DEBUG("ret=>%d", ret);
    if (!ret) // 查询成功
    {
        // 从表中检索完整的结果集
        MYSQL_RES *result = mysql_store_result(mysql_);

        while (MYSQL_ROW row = mysql_fetch_row(result))
        {

            key_vector->push_back(row[0]);
            // LOG_INFO("row=>%d", mg_id);
        }
    }
    else
    {
        return;
    }
}
