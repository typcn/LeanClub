#ifndef INCLUDE_CONFIG_H_
#define INCLUDE_CONFIG_H_

#define LC_VERSION 1.1010

#define THREAD_NUM 500 // The server is async , but database opreation is block , Increase thread num to improve server handling capacity

#define DATABASE_URL "YOUR DATABASE URL" // eg: http://127.0.0.1/
#define REDIS_MASTER_IP "REDIS MASTER IP ADDRESS" // For Write , eg: 127.0.0.1
#define REDIS_READ_IP "127.0.0.1" // For Read , eg: 127.0.0.1

#define SESSION_COOKIE_DOMAIN "leanclub.org"
#define SESSION_COOKIE_LENGTH 40
#define SESSION_COOKIE_EXPRIES 31536000 // 1 Year

#define TOPIC_PER_PAGE 20
#define REPLY_PER_PAGE 50

#define MARKDOWN_UNIT 128

#define WEBSITE_URL "https://leanclub.org/"
#define GRAVATAR_BASE_URL "https://cn.gravatar.com/avatar/"

const std::string DATABASE_USERNAME = "";
const std::string DATABASE_PASSWORD = "";

const std::string GITHUB_OAUTH_ID = "";
const std::string GITHUB_OAUTH_SECRET = "";

const std::string GOOGLE_OAUTH_ID = "";
const std::string GOOGLE_OAUTH_SECRET = "";

const std::string Twitter_OAUTH_ID = "";
const std::string Twitter_OAUTH_SECRET = "";


#endif  // INCLUDE_CONFIG_H_