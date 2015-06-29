#include "crow.h"
#include "json.h"
#include "mustache.h"

#include "api.h"
#include "config.h"
#include "utils.h"
#include "pages.h"

#include <vector>
#include <sstream>
#include <thread>
#include <time.h>
#include "redis3m/include/redis3m/redis3m.hpp"
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>


std::string category_nav;

class ExampleLogHandler : public crow::ILogHandler {
    public:
        void log(std::string message, crow::LogLevel level) override {
//            cerr << "ExampleLogHandler -> " << message;
        }
};

int main()
{
    InitCurl();
    
    InitRedisPool();
    InitOnlineCount();
    BuildCategoryIndex();
    
    crow::SimpleApp app;

    crow::mustache::set_base("./templates");

    CROW_ROUTE(app, "/")
    ([]{
        clock_t begin, end;
        begin = clock();
        double time_spent;
        crow::mustache::context ctx;
        ctx["title"] = "LeanClub";
        ctx["online"] = GetOnlineCount();
        ctx["topics"] = render_topic_list_string(get_new_topic(1,0));
        ctx["categorynav"] = category_nav;
        ctx["version"] = LC_VERSION;
        end = clock();
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        ctx["time"] = time_spent;
        return crow::mustache::load("index.html").render(ctx);
    });

    CROW_ROUTE(app, "/topic/<string>")
    ([](const crow::request& req,crow::response& res,std::string topicid){
        clock_t begin, end;
        begin = clock();
        double time_spent;
        std::string topic = get_topic(topicid);
        if(topic == "error"){
            res.code = 404;
            res.end("404 Not Found");
        }else{
            crow::json::rvalue topic_json = crow::json::load(topic);
            std::string title = topic_json["title"].s();
            crow::mustache::context ctx;
            ctx["title"] =  title + " - View Topic - LeanClub";
            ctx["topic_title"] = title;
            ctx["online"] = GetOnlineCount();
            ctx["topic"] = render_topic_json(topic_json);
            ctx["version"] = LC_VERSION;
            ctx["categorynav"] = category_nav;
            end = clock();
            time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
            ctx["time"] = time_spent;
            res.end(crow::mustache::load("topic.html").render(ctx));
        }
        res.end();
    });
    
    CROW_ROUTE(app, "/page/<int>")
    ([](const crow::request& req,crow::response& res,int page){
        clock_t begin, end;
        begin = clock();
        double time_spent;
        if(page < 0){
            res.code = 404;
            res.end("404 Not Found");
            return;
        }
        std::string topics = get_new_topic(page,0);
        if(topics == "error"){
            res.code = 404;
            res.end("404 Not Found");
            return;
        }
        crow::mustache::context ctx;
        ctx["page"] = page;
        ctx["title"] = "第" + std::to_string(page) + "页 - 最新 - LeanClub";
        ctx["online"] = GetOnlineCount();
        ctx["topics"] = render_topic_list_string(topics);
        ctx["prev"] = page - 1;
        ctx["next"] = page + 1;
        ctx["category"] = "new";
        ctx["categoryname"] = "最新";
        ctx["categorynav"] = category_nav;
        ctx["version"] = LC_VERSION;
        end = clock();
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        ctx["time"] = time_spent;
        res.end(crow::mustache::load("node.html").render(ctx));
    });

    CROW_ROUTE(app, "/node/<string>/<int>")
    ([](const crow::request& req,crow::response& res,std::string node_name,int page){
        clock_t begin, end;
        begin = clock();
        double time_spent;
        crow::json::rvalue category = GetCategoryInfoByUrl(node_name);
        if(category.t() == crow::json::type::Null){
            res.code=404;
            res.end("404 Not Found");
            return;
        }
        std::string id_str = category["id"].s();
        int id = std::stoi(id_str);
        std::string category_name = category["name"].s();
        std::string topics = get_new_topic(page,id);
        if(topics == "error"){
            res.code = 500;
            res.end("500 Internal Server Error");
            return;
        }
        crow::mustache::context ctx;
        ctx["page"] = page;
        ctx["title"] = "第" + std::to_string(page) + "页 - " + category_name + " - LeanClub";
        ctx["online"] = GetOnlineCount();
        ctx["topics"] = render_topic_list_string(topics);
        ctx["category"] = crow::json::escape(node_name);
        ctx["categoryname"] = category_name;
        ctx["categorynav"] = category_nav;
        ctx["prev"] = page - 1;
        ctx["next"] = page + 1;
        ctx["version"] = LC_VERSION;
        end = clock();
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        ctx["time"] = time_spent;
        res.end(crow::mustache::load("node.html").render(ctx));
    });
    
    CROW_ROUTE(app, "/info/avatar/<string>")
    ([](const crow::request& req,crow::response& res,std::string username){
        std::string uinfo = user_info_by_name(username);
        res.code = 302;
        res.add_header("Cache-Control", " max-age=86400");
        if(uinfo == "error"){
            res.add_header("Location", "/static/images/noavatar.png");
        }else{
            crow::json::rvalue rv = crow::json::load(uinfo);
            res.add_header("Location", rv["rows"][0]["value"]["avatar"].s());
        }
        res.end();
    });
    
    CROW_ROUTE(app, "/user/name/<string>")
    ([](const crow::request& req,crow::response& res,std::string username){
        std::string uinfo = user_info_by_name(username);
        res.add_header("Cache-Control", " max-age=86400");
        if(uinfo == "error"){
            res.code = 404;
            res.end("404 Not Found");
        }else{
            clock_t begin, end;
            begin = clock();
            double time_spent;
            crow::json::rvalue rv = crow::json::load(uinfo);
            crow::mustache::context ctx;
            ctx = rv["rows"][0]["value"];
            ctx["online"] = GetOnlineCount();
            ctx["categorynav"] = category_nav;
            ctx["version"] = LC_VERSION;
            end = clock();
            time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
            ctx["time"] = time_spent;
            res.end(crow::mustache::load("userinfo.html").render(ctx));
        }
    });
    
    CROW_ROUTE(app, "/post/new")
    ([](const crow::request& req,crow::response& res){
        clock_t begin, end;
        begin = clock();
        double time_spent;
        crow::mustache::context ctx;
        ctx["title"] = "发帖 - LeanClub";
        ctx["online"] = GetOnlineCount();
        ctx["categorynav"] = category_nav;
        ctx["version"] = LC_VERSION;
        end = clock();
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        ctx["time"] = time_spent;
        res.end(crow::mustache::load("postnew.html").render(ctx));
    });
    
    CROW_ROUTE(app, "/notifications")
    ([](const crow::request& req,crow::response& res){
        clock_t begin, end;
        begin = clock();
        double time_spent;
        std::string SESSION = GetSession(req.get_header_value("cookie"));
        if(SESSION.length() > 0){
            crow::json::rvalue JSON = crow::json::load(SESSION);
            std::string uid = JSON["uid"].s();
            crow::mustache::context ctx;
            std::string noti = get_notifications(uid);
            if(noti != "error"){
                crow::json::rvalue noti_json = crow::json::load(noti);
                ctx["notifications"] = noti_json["notifications"];
            }else{
                crow::json::wvalue noti_json;
                noti_json[0]["time"] = time(0);
                noti_json[0]["content"] = "您没有任何提醒";
                noti_json[0]["url"] = "/";
                ctx["notifications"] = crow::json::load(crow::json::dump(noti_json));
            }
            ctx["online"] = GetOnlineCount();
            ctx["categorynav"] = category_nav;
            ctx["version"] = LC_VERSION;
            end = clock();
            time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
            ctx["time"] = time_spent;
            res.end(crow::mustache::load("notifications.html").render(ctx));
            return;
        }else{
            res.code = 403;
            res.end("Session not found");
            return;
        }
    });
    
    CROW_ROUTE(app, "/settings")
    ([](const crow::request& req,crow::response& res){
        clock_t begin, end;
        begin = clock();
        double time_spent;
        crow::mustache::context ctx;
        ctx["title"] = "设置 - LeanClub";
        ctx["online"] = GetOnlineCount();
        ctx["version"] = LC_VERSION;
        end = clock();
        time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
        ctx["time"] = time_spent;
        res.end(crow::mustache::load("settings.html").render(ctx));
    });
    
    CROW_ROUTE(app, "/login/github")
    ([](const crow::request& req,crow::response& res){
        res.add_header("Location", "https://github.com/login/oauth/authorize?scope=user:email&client_id=" + GITHUB_OAUTH_ID);
        res.code = 301;
        res.end("Redirecting");
    });
    
    CROW_ROUTE(app, "/login/google")
    ([](const crow::request& req,crow::response& res){
        res.add_header("Location","https://accounts.google.com/o/oauth2/auth?"
                       "scope=email&response_type=code&client_id=" + GOOGLE_OAUTH_ID +
                       "&approval_prompt=force&redirect_uri=" WEBSITE_URL "login/callback/google");
        res.code = 302;
        res.end("Redirecting");
    });
    
    CROW_ROUTE(app, "/login/twitter")
    ([](const crow::request& req,crow::response& res){
        res.add_header("Location",oauth_get_twitter_login_url());
        res.code = 302;
        res.end("Redirecting");
    });
    
    CROW_ROUTE(app, "/login/callback/github")
    ([](const crow::request& req,crow::response& res){
        if(req.url_params.get("do") != nullptr){
            if(req.url_params.get("code") != nullptr) {
                std::string result = oauth_login_github(req.url_params.get("code"));
                if(result.length() < 40){
                    res.write("{\"error\":1,\"msg\":\"" + result +"\"}");
                    res.end();
                    return;
                }else{
                    res.add_header("Set-Cookie",SetSession(result));
                    
                    res.write("{\"error\":0,\"result\":" + result + "}");
                    res.end();
                    return;
                }
            }else{
                res.write("{\"error\":1,\"msg\":\"Auth Failed\"}");
                res.end();
                return;
            }
        }else{
            crow::mustache::context ctx;
            ctx["version"] = LC_VERSION;
            res.write(crow::mustache::load("signing-in.html").render(ctx));
            res.end();
        }
    });

    CROW_ROUTE(app, "/login/callback/google")
    ([](const crow::request& req,crow::response& res){
        if(req.url_params.get("do") != nullptr){
            if(req.url_params.get("code") != nullptr) {
                std::string result = oauth_login_google(req.url_params.get("code"));
                if(result.length() < 40){
                    res.write("{\"error\":1,\"msg\":\"" + result +"\"}");
                    res.end();
                    return;
                }else{
                    res.add_header("Set-Cookie",SetSession(result));
                    
                    res.write("{\"error\":0,\"result\":" + result + "}");
                    res.end();
                    return;
                }
            }else{
                res.write("{\"error\":1,\"msg\":\"Auth Failed\"}");
                res.end();
                return;
            }
        }else{
            crow::mustache::context ctx;
            ctx["version"] = LC_VERSION;
            res.write(crow::mustache::load("signing-in.html").render(ctx));
            res.end();
        }
    });
    
    CROW_ROUTE(app, "/login/callback/twitter")
    ([](const crow::request& req,crow::response& res){
        if(req.url_params.get("do") != nullptr){
            if(req.url_params.get("oauth_token") != nullptr) {
                std::string result = oauth_login_twitter(req.url_params.get("oauth_token"),req.url_params.get("oauth_verifier"));
                if(result.length() < 40){
                    res.write("{\"error\":1,\"msg\":\"" + result +"\"}");
                    res.end();
                    return;
                }else{
                    res.add_header("Set-Cookie",SetSession(result));
                    
                    res.write("{\"error\":0,\"result\":" + result + "}");
                    res.end();
                    return;
                }
            }else{
                res.write("{\"error\":1,\"msg\":\"Auth Failed\"}");
                res.end();
                return;
            }
        }else{
            crow::mustache::context ctx;
            ctx["version"] = LC_VERSION;
            res.write(crow::mustache::load("signing-in.html").render(ctx));
            res.end();
        }
    });
    
    CROW_ROUTE(app, "/api/post/new").methods("POST"_method)
    ([](const crow::request& req,crow::response& res){
        if(req.get_header_value("x-requested-with") != "XMLHttpRequest"){
            res.code = 403;
            res.end("Please use ajax post");
            return;
        }
        std::string Referer = req.get_header_value("referer");
        if(Referer.find("https://leanclub.org/post/new") == std::string::npos){
            res.code = 403;
            res.end("Please post on offical site");
            return;
        }
        std::string SESSION = GetSession(req.get_header_value("cookie"));
        if(SESSION.length() > 0){
            crow::json::rvalue JSON = crow::json::load(SESSION);
            std::string uname = JSON["username"].s();
            crow::json::rvalue BODY = crow::json::load(req.body);
            if(BODY["title"].size() > 80 ){
                res.end("Title too long");
                return;
            }
            if(BODY["content"].size() > 40240){
                res.end("Content too long");
                return;
            }
            std::string content = BODY["content"].s();
            crow::json::rvalue category = GetCategoryInfoByUrl(BODY["category"].s());
            int cid = std::stoi(category["id"].s());
            std::string result = post_topic(uname,BODY["title"].s(),content,cid);
            
            res.code = 200;
            res.end(result);
            check_at_notification(uname, result, content);
            return;
        }else{
            res.code = 403;
            res.end("Session not found");
            return;
        }
    });
   
    CROW_ROUTE(app, "/api/node/<string>/<int>")
    ([](const crow::request& req,crow::response& res,std::string node_name,int page){
        int id;
        if(node_name == "new"){
            id = 0;
        }else{
            crow::json::rvalue category = GetCategoryInfoByUrl(node_name);
            if(category.t() == crow::json::type::Null){
                res.code=404;
                res.end("404 Not Found");
                return;
            }
            std::string id_str = category["id"].s();
            id = std::stoi(id_str);
        }
        std::string topics = get_new_topic(page,id);
        if(topics == "error"){
            res.code=500;
            res.end("500 Internal Server Error");
            return;
        }
        res.end(crow::json::dump(render_topic_list_string(topics)));
    });

    CROW_ROUTE(app, "/api/topic/<string>")
    ([](const crow::request& req,crow::response& res,std::string topicID){
        std::string topic = get_topic(topicID);
        if(topic == "error"){
            res.code = 404;
        }
        crow::json::rvalue topic_json = crow::json::load(topic);
        res.end(crow::json::dump(render_topic_json(topic_json)));
    });
    
    CROW_ROUTE(app, "/api/notification/count")
    ([](const crow::request& req,crow::response& res){
        std::string SESSION = GetSession(req.get_header_value("cookie"));
        if(SESSION.length() > 0){
            crow::json::rvalue JSON = crow::json::load(SESSION);
            std::string uid = JSON["uid"].s();
            res.end(std::to_string(get_notification_count(uid)));
            return;
        }else{
            res.code = 403;
            res.end("Session not found");
            return;
        }
    });
    
    CROW_ROUTE(app, "/api/notification/clear")
    ([](const crow::request& req,crow::response& res){
        std::string SESSION = GetSession(req.get_header_value("cookie"));
        if(SESSION.length() > 0){
            crow::json::rvalue JSON = crow::json::load(SESSION);
            std::string uid = JSON["uid"].s();
            clear_notification(uid);
            res.code = 302;
            res.add_header("Location", "/notifications");
            res.end("clear");
            return;
        }else{
            res.code = 403;
            res.end("Session not found");
            return;
        }
    });
    
    CROW_ROUTE(app, "/api/post/reply/<string>").methods("POST"_method)
    ([](const crow::request& req,crow::response& res,std::string topicID){
        if(req.get_header_value("x-requested-with") != "XMLHttpRequest"){
            res.code = 403;
            res.end("Please use ajax post");
            return;
        }
        std::string Referer = req.get_header_value("referer");
        if(Referer.find("https://leanclub.org/topic/") == std::string::npos){
            res.code = 403;
            res.end("Please post on offical site");
            return;
        }
        if(req.body.size() > 10240){
            res.end("Content too long");
            return;
        }
        std::string SESSION = GetSession(req.get_header_value("cookie"));
        if(SESSION.length() > 0){
            crow::json::rvalue JSON = crow::json::load(SESSION);
            std::string uname = JSON["username"].s();
            add_reply(uname,req.body,topicID);
            res.code = 200;
            res.end("Post success");
            check_at_notification(uname, topicID, req.body);
            return;
        }else{
            res.code = 403;
            res.end("Session not found");
            return;
        }
    });
    
    CROW_ROUTE(app, "/api/user/changeName/<string>")
    ([](const crow::request& req,crow::response& res,std::string newName){
        if(req.get_header_value("x-requested-with") != "XMLHttpRequest"){
            res.code = 403;
            res.end("Please use ajax post");
            return;
        }
        std::string Referer = req.get_header_value("referer");
        if(Referer.find("https://leanclub.org/settings") == std::string::npos){
            res.code = 403;
            res.end("Please post on offical site");
            return;
        }
        std::string SESSION = GetSession(req.get_header_value("cookie"));
        if(SESSION.length() > 0){
            crow::json::rvalue JSON = crow::json::load(SESSION);
            std::string uid = JSON["uid"].s();
            res.code = 200;
            res.end(change_username(uid,newName));
            return;
        }else{
            res.code = 403;
            res.end("Session not found");
            return;
        }
    });
    
    CROW_ROUTE(app, "/api/user/changeAvatar")
    ([](const crow::request& req,crow::response& res){
        if(req.get_header_value("x-requested-with") != "XMLHttpRequest"){
            res.code = 403;
            res.end("Please use ajax post");
            return;
        }
        std::string Referer = req.get_header_value("referer");
        if(Referer.find("https://leanclub.org/settings") == std::string::npos){
            res.code = 403;
            res.end("Please post on offical site");
            return;
        }
        std::string SESSION = GetSession(req.get_header_value("cookie"));
        if(SESSION.length() > 0){
            crow::json::rvalue JSON = crow::json::load(SESSION);
            std::string uid = JSON["uid"].s();
            res.code = 200;
            res.end(change_avatar(uid, GetGravatar(req.url_params.get("email")) ));
            return;
        }else{
            res.code = 403;
            res.end("Session not found");
            return;
        }
    });
    
    CROW_ROUTE(app, "/api/file/simpleUpload").methods("POST"_method)
    ([](const crow::request& req,crow::response& res){
        if(req.get_header_value("x-requested-with") != "XMLHttpRequest"){
            res.code = 403;
            res.end("Please use ajax post");
            return;
        }
        std::string SESSION = GetSession(req.get_header_value("cookie"));
        if(SESSION.length() > 0){
            crow::json::rvalue JSON = crow::json::load(SESSION);
            std::string uid = JSON["uid"].s();
            std::string r = SavePostFile(req.body, uid);
            if(r == "error"){
                res.code = 500;
            }else{
                res.code = 200;
            }
        
            res.end(r);
            return;
        }else{
            res.code = 403;
            res.end("Session not found");
            return;
        }
    });
    
    CROW_ROUTE(app, "/faq")
    ([](const crow::request& req,crow::response& res){
        res.add_header("Location", "/static/faq.html");
        res.code = 301;
        res.end("Redirecting");
    });
    CROW_ROUTE(app, "/privacy")
    ([](const crow::request& req,crow::response& res){
        res.add_header("Location", "/static/privacy.html");
        res.code = 301;
        res.end("Redirecting");
    });
    CROW_ROUTE(app, "/about")
    ([](const crow::request& req,crow::response& res){
        res.add_header("Location", "/static/about.html");
        res.code = 301;
        res.end("Redirecting");
    });
    CROW_ROUTE(app, "/Success")
    ([](const crow::request& req,crow::response& res){
        res.add_header("Location", "/static/images/noavatar.png");
        res.code = 301;
        res.end("Redirecting");
    });
    crow::logger::setLogLevel(crow::LogLevel::WARNING);
    //crow::logger::setHandler(std::make_shared<ExampleLogHandler>());
    std::cout << "APP Listening on port 18080 with " + std::to_string(THREAD_NUM) + " thread(s)" << std::endl;
    app.port(18080)
        .multithreaded(THREAD_NUM)
        .run();
}