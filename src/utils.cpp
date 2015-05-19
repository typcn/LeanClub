//
//  utils.cpp
//  leanclub
//
//  Created by TYPCN on 2015/5/1.
//
//

#include "utils.h"
#include "config.h"
#include "restclient/restclient.h"
#include "oauth/md5.h"

#include <redis3m/redis3m.hpp>
#include <boost/thread/thread.hpp>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace redis3m;

simple_pool::ptr_t pool;
simple_pool::ptr_t WritePool;

int online = -1;

void InitRedisPool(){
    pool = simple_pool::create("127.0.0.1");
    WritePool = simple_pool::create(REDIS_MASTER_IP);
}

std::string randomStr(int len){
    std::string chars("ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890");
    boost::random::random_device rng;
    boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
    std::string str;
    for(int i = 0; i < len; ++i) {
        str = str + chars[index_dist(rng)];
    }
    return str;
}

bool string_is_valid(const std::string &str)
{
    return find_if(str.begin(), str.end(),
                   [](char c) { return !(isalnum(c)); }) == str.end();
}

char easytolower(char in){
    if(in<='Z' && in>='A')
        return in-('Z'-'z');
    return in;
}

std::string GetGravatar(std::string email){
    std::string data = email;
    std::transform(data.begin(), data.end(), data.begin(), easytolower);
    return GRAVATAR_BASE_URL + md5(data) + "?d=identicon";
}

std::string cookie_expries_time(time_t timestamp){
    std::ostringstream ss;
    ss.exceptions(std::ios_base::failbit);

    boost::posix_time::ptime ptime = boost::posix_time::from_time_t(timestamp);
    boost::posix_time::time_facet *facet = new boost::posix_time::time_facet("%a, %d-%b-%Y %T GMT");
    ss.imbue(std::locale(std::locale::classic(), facet));
    ss.str("");
    ss << ptime;
    return ss.str();
}

std::string SetSession(std::string content){
    
    std::string sessionID = randomStr(SESSION_COOKIE_LENGTH);
    connection::ptr_t c = WritePool->get();
    c->run(command("SET")("SESS" + sessionID)(content));
    WritePool->put(c);
    std::string expries = cookie_expries_time(time(0) + (SESSION_COOKIE_EXPRIES));
    return "LBSESSIONID=" + sessionID +
    "; expires="+ expries +
    "; path=/; domain=" SESSION_COOKIE_DOMAIN "; HttpOnly; Secure";
}

std::string GetSession(std::string cookie){
    size_t loc = cookie.find("LBSESSIONID=");
    if(loc == std::string::npos){
        return "";
    }else{
        std::string sessid = cookie.substr(loc + 12,SESSION_COOKIE_LENGTH);
        connection::ptr_t c = pool->get();
        std::string session = c->run(command("GET")("SESS" + sessid)).str();
        pool->put(c);
        return session;
    }
}

void UpdateOnlineCount(){
    while(1){
        RestClient::response onlineinfo = RestClient::get("https://script.google.com/macros/s/AKfycbxQ0XJpdE669DlvQ9vSWGFp1iIoQsoqEnUBW_0K2sMaZ5lxazI/exec",false);
        if(onlineinfo.code == 200){
            online = std::stoi(onlineinfo.body);
            if(online == 0){
                online = 1;
            }
        }else{
            online = -1;
        }

        boost::this_thread::sleep(boost::posix_time::seconds(300));
    }
}

void InitOnlineCount(){
    boost::thread t(&UpdateOnlineCount);
    t.detach();
}

int GetOnlineCount(){
    return online;
}
