#include "logic.h"

#define WRITE_BUFFER_SIZE 4096
#define SECRET_KEY "3MnJb57tW9TAvkYFQEDUgLdSRuBzmXcZ"

// 获取索引值
std::string Logic::getToken(int mg_id)
{
    return jwt::create()
        .set_issuer("auth0")
        .set_type("JWS")
        .set_payload_claim("mg_id", jwt::claim(std::to_string(mg_id)))
        .sign(jwt::algorithm::hs256{SECRET_KEY});
}

// 验证token并且解析其中的用户id
bool Logic::checkToken(std::string token, int &mg_id)
{
    auto decoded = jwt::decode(token);
    mg_id = -1;
    auto verifier = jwt::verify()
                        .allow_algorithm(jwt::algorithm::hs256{SECRET_KEY})
                        .with_issuer("auth0");
    try
    {
        verifier.verify(decoded);
        for (auto i : decoded.get_payload_json())
        {
            if (i.first == "mg_id")
            {
                mg_id = stoi(i.second.to_str());
            }
        }
        return true;
    }
    catch (const std::exception &e)
    {
        LOG_INFO("tokken error for %s", e.what());
        return false;
    }
}

// 登录
void Logic::loginLogic(char *user_data)
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
        errorLogic(404, "登录失败");
        return;
    }

   cpyJson2Buff(&ret_root);
}

// 菜单
void Logic::menuLogic()
{
    // 先验证token
    if (!is_token_vaild_)
    {
        errorLogic(403, "token验证不通过");
        return;
    }

    std::string sql_string("SELECT * FROM sp_permission_api as api LEFT JOIN sp_permission as main ON main.ps_id = api.ps_id WHERE main.ps_id is not null;");

    Json::Value ret_root;
    Json::Value data;
    Json::Value temp;
    Json::Value meta;
   
    int level0_num = 0;
    int mg_id = -1;
    // m_lock.lock();
    if (mysql_ == NULL)
        LOG_INFO("mysql is NULL!");

    std::shared_ptr<std::map<int, Json::Value>> level1_data = std::make_shared<std::map<int, Json::Value>>();
    
    // 在获取前先清除
    clearTableKey();
    getTableKey("sp_permission_api");
    getTableKey("sp_permission");

    int ret = mysql_query(mysql_, sql_string.c_str());
    if (!ret) // 查询成功
    {
        // 从表中检索完整的结果集
        MYSQL_RES *result = mysql_store_result(mysql_);

        while (MYSQL_ROW row = mysql_fetch_row(result))
        {
            temp["id"] = row[indexOf("ps_id")];
            temp["authName"] = row[indexOf("ps_name")];
            temp["path"] = row[indexOf("ps_api_path")];
            // temp["children"] = {}; // debug: 要用[]?
            int pid = stoi(row[indexOf("ps_pid")]);
            // temp["order"] = permission.ps_api_order
            // 根据 level判断
            // 一级菜单
            if (strncasecmp(row[indexOf("ps_level")], "0", 1) == 0)
            {
                level0_num++;
                ret_root["data"]
                    .append(temp);
            }
            // 二级菜单
            else if (strncasecmp(row[indexOf("ps_level")], "1", 1) == 0)
            {
                if (pid)
                    (*level1_data)[pid].append(temp);
            }
            temp.clear();
        }

        // 遍历ret_root中data

        for (Json::Value &json_value : ret_root["data"])
        {
            LOG_DEBUG("pid = %s", json_value["id"].asString().c_str());
            json_value["children"] = (*level1_data)[stoi(json_value["id"].asString().c_str())];
        }

        meta["msg"] = "登录成功";
        meta["status"] = 200;
        ret_root["meta"] = meta;
    }
    else
    {
        errorLogic(404, "获取目录列表失败");
        return;
    }

    cpyJson2Buff(&ret_root);
}

// 获取表的所有键的名字
void Logic::getTableKey(string table_name)
{

    std::string sql_string("SHOW COLUMNS from ");
    sql_string += table_name + " ;";

    if (mysql_ == NULL)
        LOG_INFO("mysql is NULL!");

    int ret = mysql_query(mysql_, sql_string.c_str());

    // LOG_DEBUG("ret=>%d", ret);
    if (!ret) // 查询成功de
    {
        // 从表中检索完整的结果集
        MYSQL_RES *result = mysql_store_result(mysql_);

        while (MYSQL_ROW row = mysql_fetch_row(result))
        {

            key_vector_->push_back(row[0]);
            // LOG_INFO("row=>%d", mg_id);
        }
    }
    else
    {
        return;
    }
}

// 清除存储键名的容器
void Logic::clearTableKey(){
    key_vector_->clear();
}

// 错误处理
void Logic::errorLogic(int status, std::string msg)
{
    Json::Value ret_root;
    Json::Value data;
    Json::Value meta;
    

    data["userid"] = user_id_;
    meta["msg"] = msg;
    meta["status"] = status;
    ret_root["data"] = data;
    ret_root["meta"] = meta;

    cpyJson2Buff(&ret_root);
}

// 将结果的json对象写入到暂存数组中
void Logic::cpyJson2Buff(Json::Value* ret_root){
    Json::StreamWriterBuilder writer;

      // 清空
    memset(temp_buff_, '\0', WRITE_BUFFER_SIZE);

    // 将 JSON 对象转换为字符串
    std::string jsonString = Json::writeString(writer, ret_root);

    *len_ = jsonString.size();
    // LOG_DEBUG("json_string = %s, len = %d", jsonString.c_str(), len);
    if (*len_ <= WRITE_BUFFER_SIZE)
    {
        strncpy(temp_buff_, jsonString.c_str(), *len_);
        // LOG_DEBUG("ret_json=>%s", temp_buff);
    }
}

// 获取键名对应的索引值
int Logic::indexOf(string key_name)
{
    for (int i = 0; i < key_vector_->size(); i++)
    {
        if (key_vector_->at(i) == key_name)
        {
            return i;
        }
    }
}