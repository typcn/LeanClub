//
//  api.cpp
//  leanclub
//
//  Created by TYPCN on 2015/4/30.
//
//

#include "api.h"
#include "config.h"
#include "json.h"
#include "utils.h"
#include "oauth/sha1.h"
#include "oauth/liboauthcpp.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <boost/regex.hpp>
#include <iostream>

using namespace std;



// User

string oauth_get_twitter_login_url(){
    OAuth::Consumer consumer(Twitter_OAUTH_ID, Twitter_OAUTH_SECRET);
    OAuth::Client oauth(&consumer);
    std::string oAuthQueryString =
    oauth.getURLQueryString( OAuth::Http::Get, "https://api.twitter.com/oauth/request_token?oauth_callback=" WEBSITE_URL "login/callback/twitter");
    RestClient::response res = RestClient::get("https://api.twitter.com/oauth/request_token?" + oAuthQueryString, false);
    if(res.code == 200){
        if(res.body.length() > 44){
            std::string token = res.body.substr(0,44);
            return "https://api.twitter.com/oauth/authenticate?" + token;
        }else{
            return "error";
        }
    }else{
        return "error";
    }
}

string oauth_login_github(string code){
    RestClient::response params = RestClient::post("https://github.com/login/oauth/access_token", "application/x-www-form-urlencoded", "client_id=" + GITHUB_OAUTH_ID +"&client_secret=" + GITHUB_OAUTH_SECRET + "&code=" + code,false);
    if(params.code == 200 && params.body.find("error_uri") == string::npos){
        RestClient::response userinfo = RestClient::get("https://api.github.com/user?" + params.body,false);
        if(userinfo.code == 200){
            rapidjson::Document d;
            d.Parse(userinfo.body.c_str());
            if(!d["id"].IsNull() && d["login"].IsString()){
                string uid= "github_" + to_string(d["id"].GetInt());
                CSHA1 sha1;
                sha1.Update((UINT_8*)uid.c_str(), uid.length() * sizeof(TCHAR));
                sha1.Final();
                sha1.ReportHashStl(uid,CSHA1::REPORT_HEX_SHORT);
                
                string avatar = "/static/images/no-avatar.png";
                if(d["email"].IsString()){
                    string email = d["email"].GetString();
                    avatar = GetGravatar(email);
                }
                
                string uinfo = user_info(uid);
                if(uinfo == "error"){
                    string username = d["login"].GetString();
                    username = "github_" + username;
                    
                    crow::json::wvalue result;
                    result["uid"] = uid;
                    result["username"] = username;
                    result["avatar"] = avatar;
                    result["type"] = 1;
                    result["status"] = 0;
                    result["regTime"] = time(0);
                    result["lastLogin"] = time(0);
                    result["points"] = 0;
                    string json = crow::json::dump(result);
                    RestClient::response reg = RestClient::put(DATABASE_URL"user/" + uid, "application/json", json);
                    if(reg.code == 201){
                        return json;
                    }else{
                        return "register failed";
                    }
                }else{
                    rapidjson::Document user;
                    user.Parse(uinfo.c_str());
                    crow::json::wvalue result;
                    result["uid"] = uid;
                    result["username"] = user["username"].GetString();
                    result["avatar"] = user["avatar"].GetString();
                    return crow::json::dump(result);
                }
            }else{
                return "Unable to get user id";
            }
        }else{
            return "Cannot get user info";
        }
    }else{
        return "Auth failed Please re-login";
    }
}

string oauth_login_google(string code){
    RestClient::response params = RestClient::post("https://www.googleapis.com/oauth2/v3/token", "application/x-www-form-urlencoded", "client_id=" + GOOGLE_OAUTH_ID +"&client_secret=" + GOOGLE_OAUTH_SECRET + "&code=" + code + "&grant_type=authorization_code&redirect_uri=" WEBSITE_URL "login/callback/google",false);
    if(params.code == 200 && params.body.find("error") == string::npos){
        rapidjson::Document params_json;
        params_json.Parse(params.body.c_str());
        string access_token = params_json["access_token"].GetString();
        RestClient::response userinfo = RestClient::get("https://www.googleapis.com/userinfo/v2/me?access_token="
                                                        + access_token,false);
        if(userinfo.code == 200){
            rapidjson::Document d;
            d.Parse(userinfo.body.c_str());
            
            if(!d["id"].IsNull() && d["email"].IsString()){
                string google_id = d["id"].GetString();
                string email = d["email"].GetString();
                string uid= "google_"+ google_id;
                CSHA1 sha1;
                sha1.Update((UINT_8*)uid.c_str(), uid.length() * sizeof(TCHAR));
                sha1.Final();
                sha1.ReportHashStl(uid,CSHA1::REPORT_HEX_SHORT);
                
                string avatar = GetGravatar(email);
                
                string uinfo = user_info(uid);
                if(uinfo == "error"){
                    string username = "google_" + google_id;
                    
                    crow::json::wvalue result;
                    result["uid"] = uid;
                    result["username"] = username;
                    result["avatar"] = avatar;
                    result["type"] = 1;
                    result["status"] = 0;
                    result["regTime"] = time(0);
                    result["lastLogin"] = time(0);
                    result["points"] = 0;
                    string json = crow::json::dump(result);
                    RestClient::response reg = RestClient::put(DATABASE_URL"user/" + uid, "application/json", json);
                    if(reg.code == 201){
                        return json;
                    }else{
                        return "register failed";
                    }
                }else{
                    rapidjson::Document user;
                    user.Parse(uinfo.c_str());
                    crow::json::wvalue result;
                    result["uid"] = uid;
                    result["username"] = user["username"].GetString();
                    result["avatar"] = user["avatar"].GetString();
                    return crow::json::dump(result);
                }
            }else{
                return "Unable to get user id";
            }
        }else{
            return "Cannot get user info";
        }
    }else{
        return "Auth failed";
    }
}

string oauth_login_twitter(string token,string verifier){
    OAuth::Consumer consumer(Twitter_OAUTH_ID, Twitter_OAUTH_SECRET);
    OAuth::Client oauth(&consumer);
    std::string oAuthQueryString =
    oauth.getURLQueryString( OAuth::Http::Post, "https://api.twitter.com/oauth/access_token?oauth_token="
                            + token,"oauth_verifier=" + verifier);
    
    RestClient::response params = RestClient::get("https://api.twitter.com/oauth/access_token?" + oAuthQueryString,false);
    if(params.code == 200 && params.body.find("user_id") != string::npos){
        OAuth::KeyValuePairs access_token_resp_data = OAuth::ParseKeyValuePairs(params.body);
        string uid;
        std::pair<OAuth::KeyValuePairs::iterator, OAuth::KeyValuePairs::iterator> screen_name_its = access_token_resp_data.equal_range("user_id");
        for(OAuth::KeyValuePairs::iterator it = screen_name_its.first; it != screen_name_its.second; it++)
            uid = it->second;
        
        if(uid.length() > 0){
            string uid_hash = "";
            CSHA1 sha1;
            sha1.Update((UINT_8*)uid.c_str(), uid.length() * sizeof(TCHAR));
            sha1.Final();
            sha1.ReportHashStl(uid_hash,CSHA1::REPORT_HEX_SHORT);
            
            string uinfo = user_info(uid_hash);
            if(uinfo == "error"){
                string username = "twitter_" + uid;
                
                crow::json::wvalue result;
                result["uid"] = uid_hash;
                result["username"] = username;
                result["avatar"] = "/static/images/noavatar.png";
                result["type"] = 1;
                result["status"] = 0;
                result["regTime"] = time(0);
                result["lastLogin"] = time(0);
                result["points"] = 0;
                string json = crow::json::dump(result);
                RestClient::response reg = RestClient::put(DATABASE_URL"user/" + uid_hash, "application/json", json);
                if(reg.code == 201){
                    return json;
                }else{
                    return "register failed";
                }
            }else{
                rapidjson::Document user;
                user.Parse(uinfo.c_str());
                crow::json::wvalue result;
                result["uid"] = uid_hash;
                result["username"] = user["username"].GetString();
                result["avatar"] = user["avatar"].GetString();
                return crow::json::dump(result);
            }
            
        }else{
            return "Unable to get user id";
        }
    }else{
        return "Auth failed";
    }
}

string user_info(string uid){
    RestClient::response r = RestClient::get(DATABASE_URL"user/" + uid,true);
    if(r.code == 200){
        return r.body;
    }else{
        return "error";
    }
}

string get_notifications(string uid){
    RestClient::response r = RestClient::get(DATABASE_URL"notification/" + uid,true);
    if(r.code == 200){
        return r.body;
    }else{
        return "error";
    }
}

void check_at_notification(string uname,string url,string content){
    boost::regex reg("@(.*?) ");
    boost::smatch match;
    boost::sregex_token_iterator iter(content.begin(), content.end(), reg, 1);
    boost::sregex_token_iterator end;
    for( ; iter != end; ++iter ) {
        string uinfo = user_info_by_name(*iter);
        if(uinfo != "error"){
            crow::json::rvalue rv = crow::json::load(uinfo);
            string uid = rv["rows"][0]["value"]["uid"].s();
            add_notification(uid, uname, "在帖子中提到了您","https://leanclub.org/topic/" + url);
        }
    }
}

string add_notification(string uid,string fromname,string content,string link){
    crow::json::wvalue post;
    post["user"] = fromname;
    post["content"] = content;
    post["link"] = link;
    RestClient::response r = RestClient::put(DATABASE_URL"notification/_design/update/_update/addNotification/" + uid,"text/plain", crow::json::dump(post));
    if(r.code == 200 || r.code == 201){
        return r.body;
    }else{
        return "error";
    }
}

int get_notification_count(string uid){
    RestClient::response r = RestClient::get(DATABASE_URL"notification/_design/get/_view/count?key=\"" + uid + "\"",true);
    if(r.code == 200){
        if(r.body.find("rows\":[\r\n\r\n]") == string::npos){
            return crow::json::load(r.body)["rows"][0]["value"].i();
        }else{
            return 0;
        }
    }else{
        return 0;
    }
}

void clear_notification(string uid){
    RestClient::put(DATABASE_URL"notification/_design/update/_update/removeAll/" + uid,"text/plain", "");
}

string user_info_by_name(string uname){
    RestClient::response r = RestClient::get(DATABASE_URL"user/_design/user/_view/username?key=\"" + uname + "\"",true);
    if(r.code == 200){
        if(r.body.find("rows\":[\r\n\r\n]") == string::npos){
            return r.body;
        }else{
            return "error";
        }
    }else{
        return "error";
    }
}

// Topic

string search_db(string keyword){
    // return : search result json
    return "";
}

string post_topic(string username,string title,string content,int category){
    crow::json::wvalue post;
    post["user"] = username;
    post["title"] = title;
    post["category"] = to_string(category);
    post["status"] = 1;
    post["content"][0] = content;
    post["createAt"] = time(0);
    post["updateAt"] = 0;
    post["lastReply"] = time(0);
    RestClient::response r = RestClient::post(DATABASE_URL"topics", "application/json", crow::json::dump(post), true);
    rapidjson::Document result;
    result.Parse(r.body.c_str());
    if(result["id"].IsString()){
        return result["id"].GetString();
    }else{
        if(result["reason"].IsString()){
            return result["reason"].GetString();
        }else{
            return "post failed";
        }
    }
}

string add_reply(string username,string content,string tid){
    crow::json::wvalue post;
    post["user"] = username;
    post["body"] = content;
    RestClient::response r = RestClient::put(DATABASE_URL"topics/_design/update/_update/addReplyX/" + tid,"text/plain", crow::json::dump(post));
    if(r.code == 200){
        return "success";
    }else{
        return "error";
    }
}

string change_username(string uid,string username){
    if(user_info_by_name(username) == "error"){
        boost::regex r("[a-zA-Z0-9_]+");  // At least one character in a-z or A-Z ranges
        bool match = boost::regex_match(username, r);
        if (match){
            RestClient::response r = RestClient::put(DATABASE_URL"user/_design/update/_update/changeName/" + uid + "?username=" + username, "text/plain", "");
            if(r.code == 200 && r.body == "Success"){
                return "success";
            }else{
                return r.body;
            }
        }
        else{
            return "必须为字母或数字";
        }
    }
    return "Username already exits";
}

string change_avatar(string uid,string avatar){
    RestClient::response r = RestClient::put(DATABASE_URL"user/_design/update/_update/changeAvatar/" + uid + "?avatar=" + avatar, "text/plain", "");
    if(r.code == 200 && r.body == "Success"){
        return "success";
    }else{
        return r.body;
    }
}

string get_topic(string ID){
    RestClient::response r = RestClient::get(DATABASE_URL"topics/" + ID,true);
    if(r.code == 200){
        return r.body;
    }else{
        return "error";
    }
}

string get_new_topic(int page,int category){
    string c = to_string(category);
    if(category == 0){
        c = "new";
    }
    int skip = (page - 1) * TOPIC_PER_PAGE;
    RestClient::response r = RestClient::get(DATABASE_URL"topics/_design/get/_view/" + c + "?descending=true&limit=" + to_string(TOPIC_PER_PAGE) + "&skip=" + to_string(skip),true);
    if(r.code == 200){
        if(r.body.find("rows\":[\r\n\r\n]") == string::npos){
            return r.body;
        }else{
            return "error";
        }
    }else{
        return "error";
    }
}

// Reply

string post_reply(string postID,string content,int category){
    // return : topic ID
    return "";
}
