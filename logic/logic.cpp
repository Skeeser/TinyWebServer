#include "logic.h"

#define WRITE_BUFFER_SIZE 1024
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

    std::string sql_string("SELECT * FROM sp_manager WHRER mg_name = ");
    sql_string += root["username"].asString();
    sql_string += "AND mg_pwd =";
    sql_string += root["password"].asString() + ";";

    Json::Value ret_root;
    Json::Value data;
    Json::Value meta;
    int  mg_id = -1;
    if (!mysql_query(mysql_, sql_string.c_str())) // 查询成功
    {
        // 从表中检索完整的结果集
        MYSQL_RES *result = mysql_store_result(mysql_);

        while (MYSQL_ROW row = mysql_fetch_row(result))
        {
            LOG_INFO("row=>%s", row);
            mg_id = std::stoi(row[0]);
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
    memset(user_data, '\0', WRITE_BUFFER_SIZE);

    // 将 JSON 对象转换为字符串
    std::string jsonString = Json::writeString(writer, ret_root);
    len = jsonString.size();
    if (len <= WRITE_BUFFER_SIZE)
    {
        strncpy(user_data, jsonString.c_str(), len);
    }
}
