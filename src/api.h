//
//  api.h
//  leanclub
//
//  Created by TYPCN on 2015/4/30.
//
//

#ifndef __leanclub__api__
#define __leanclub__api__

#include "stdio.h"
#include "restclient/restclient.h"

std::string oauth_get_twitter_login_url();

std::string oauth_login_github(std::string code);
std::string oauth_login_google(std::string code);
std::string oauth_login_twitter(std::string token,std::string verifier);

std::string user_info(std::string uid);
std::string user_info_by_name(std::string uname);

std::string post_topic(std::string uid,std::string title,std::string content,int category);
std::string add_reply(std::string username,std::string content,std::string tid);

std::string change_username(std::string uid,std::string username);
std::string change_avatar(std::string uid,std::string avatar);

std::string get_new_topic(int page,int category);
std::string get_topic(std::string ID);

void check_at_notification(std::string uname,std::string url,std::string content);
void clear_notification(std::string uid);

std::string add_notification(std::string uid,std::string fromname,std::string content,std::string link);
std::string get_notifications(std::string uid);

int get_notification_count(std::string uid);

#endif /* defined(__leanclub__api__) */
