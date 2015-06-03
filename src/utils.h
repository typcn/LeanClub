//
//  utils.h
//  leanclub
//
//  Created by TYPCN on 2015/5/1.
//
//

#ifndef __leanclub__utils__
#define __leanclub__utils__

#include <string>

std::string randomStr(int len);
std::string ReplaceAll(std::string str, const std::string& from, const std::string& to);
bool string_is_valid(const std::string &str);
std::string GetGravatar(std::string email);
std::string cookie_expries_time(time_t timestamp);
std::string SetSession(std::string content);
std::string GetSession(std::string cookie);
void InitOnlineCount();
int GetOnlineCount();
void InitRedisPool();

#endif /* defined(__leanclub__utils__) */
