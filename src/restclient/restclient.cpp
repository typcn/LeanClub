/**
 * @file restclient.cpp
 * @brief implementation of the restclient class
 * @author Daniel Schauenberg <d@unwiredcouch.com>
 */

/*========================
         INCLUDES
  ========================*/
#include "restclient.h"
#include "../config.h"
#include <cstring>
#include <string>
#include <iostream>
#include <map>
#include <set>
#include <mutex>

/** initialize user agent string */
const char* RestClient::user_agent = "LeanClubForum/0.1 TYPCNServerContainer/1.0 restclient-cpp/" VERSION;
/** initialize authentication variable */
std::string RestClient::user_pass =  DATABASE_USERNAME + ":" +DATABASE_PASSWORD;
/** Authentication Methods implementation */
void RestClient::clearAuth(){
  RestClient::user_pass.clear();
}
void RestClient::setAuth(const std::string& user,const std::string& password){
  RestClient::user_pass.clear();
  RestClient::user_pass += user+":"+password;
}


// Curl handle pool

std::set<CURL *> connections;
std::mutex access_mutex;

CURL *GetCurlHandle(){
    CURL *ret = NULL;
    access_mutex.lock();
    std::set<CURL *>::iterator it = connections.begin();
    if (it != connections.end())
    {
        ret = *it;
        connections.erase(it);
    }
    access_mutex.unlock();
    if (!ret)
    {
        ret = curl_easy_init();
    }
    return ret;
}

void PutCurlHandle(CURL * curl)
{
    if (curl)
    {
        std::unique_lock<std::mutex> lock(access_mutex);
        connections.insert(curl);
    }
}

// openssl lock

#ifdef USE_OPENSSL
#include <openssl/crypto.h>
static void lock_callback(int mode, int type, char *file, int line)
{
    (void)file;
    (void)line;
    if (mode & CRYPTO_LOCK) {
        pthread_mutex_lock(&(lockarray[type]));
    }
    else {
        pthread_mutex_unlock(&(lockarray[type]));
    }
}

static unsigned long thread_id(void)
{
    unsigned long ret;
    
    ret=(unsigned long)pthread_self();
    return(ret);
}

static void init_locks(void)
{
    int i;
    
    lockarray=(pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() *
                                                sizeof(pthread_mutex_t));
    for (i=0; i<CRYPTO_num_locks(); i++) {
        pthread_mutex_init(&(lockarray[i]),NULL);
    }
    
    CRYPTO_set_id_callback((unsigned long (*)())thread_id);
    CRYPTO_set_locking_callback((void (*)())lock_callback);
}

static void kill_locks(void)
{
//    int i;
    
//    CRYPTO_set_locking_callback(NULL);
//    for (i=0; i<CRYPTO_num_locks(); i++)
//        pthread_mutex_destroy(&(lockarray[i]));
//    
//    OPENSSL_free(lockarray);
}
#endif

void InitCurl(){
    curl_global_init(CURL_GLOBAL_ALL);
    #ifdef USE_OPENSSL
        init_locks();
    #endif
}

/**
 * @brief HTTP GET method
 *
 * @param url to query
 *
 * @return response struct
 */
RestClient::response RestClient::get(const std::string& url,bool auth)
{
  /** create return struct */
  RestClient::response ret = {};

  // use libcurl
  CURL *curl = NULL;
  CURLcode res = CURLE_OK;

  curl = GetCurlHandle();
  if (curl)
  {
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    /** set basic authentication if present*/
    if(auth){
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(curl, CURLOPT_USERPWD, RestClient::user_pass.c_str());
    }else{
        curl_easy_setopt(curl,CURLOPT_HTTPAUTH,CURLAUTH_NONE);
    }
    /** set user agent */
    curl_easy_setopt(curl, CURLOPT_USERAGENT, RestClient::user_agent);
    /** set query URL */
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    /** set callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RestClient::write_callback);
    /** set data object to pass to callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ret);
    /** set the header callback function */
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, RestClient::header_callback);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    /* keep-alive idle time to 120 seconds */
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 1200L);
    /* interval time between keep-alive probes: 60 seconds */
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 60L);
    /** callback object for headers */
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &ret);
    /** perform the actual query */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        ret.body = "Failed to query.";
        ret.code = -1;
    }else{
        long http_code = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
        ret.code = static_cast<int>(http_code);
    }
    
    if(url.find("https://") == std::string::npos){
        PutCurlHandle(curl);
    }else{
        curl_easy_cleanup(curl);
    }
  }
  return ret;
}
/**
 * @brief HTTP POST method
 *
 * @param url to query
 * @param ctype content type as string
 * @param data HTTP POST body
 *
 * @return response struct
 */
RestClient::response RestClient::post(const std::string& url,
                                      const std::string& ctype,
                                      const std::string& data,bool auth)
{
  /** create return struct */
  RestClient::response ret = {};
  /** build content-type header string */
  std::string ctype_header = "Content-Type: " + ctype;

  // use libcurl
  CURL *curl = NULL;
  CURLcode res = CURLE_OK;

  curl = GetCurlHandle();
  if (curl)
  {
      curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    /** set basic authentication if present*/
    if(auth){
        curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
        curl_easy_setopt(curl, CURLOPT_USERPWD, RestClient::user_pass.c_str());
    }else{
        curl_easy_setopt(curl,CURLOPT_HTTPAUTH,CURLAUTH_NONE);
    }
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
      /* keep-alive idle time to 120 seconds */
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 1200L);
      /* interval time between keep-alive probes: 60 seconds */
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 60L);
    /** set user agent */
    curl_easy_setopt(curl, CURLOPT_USERAGENT, RestClient::user_agent);
    /** set query URL */
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    /** Now specify we want to POST data */
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    /** set post fields */
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, data.size());
    /** set callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RestClient::write_callback);
    /** set data object to pass to callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ret);
    /** set the header callback function */
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, RestClient::header_callback);
    /** callback object for headers */
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &ret);
    /** set content-type header */
    curl_slist* header = NULL;
    header = curl_slist_append(header, ctype_header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    /** perform the actual query */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
      ret.body = "Failed to query.";
      ret.code = -1;
      return ret;
    }
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    ret.code = static_cast<int>(http_code);

    curl_slist_free_all(header);
    curl_easy_cleanup(curl);
  }

  return ret;
}
/**
 * @brief HTTP PUT method
 *
 * @param url to query
 * @param ctype content type as string
 * @param data HTTP PUT body
 *
 * @return response struct
 */
RestClient::response RestClient::put(const std::string& url,
                                     const std::string& ctype,
                                     const std::string& data)
{
  /** create return struct */
  RestClient::response ret = {};
  /** build content-type header string */
  std::string ctype_header = "Content-Type: " + ctype;

  /** initialize upload object */
  RestClient::upload_object up_obj;
  up_obj.data = data.c_str();
  up_obj.length = data.size();

  // use libcurl
  CURL *curl = curl_easy_init();
  CURLcode res = CURLE_OK;

  curl = GetCurlHandle();
  if (curl)
  {
      curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
      /* keep-alive idle time to 120 seconds */
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 1200L);
      /* interval time between keep-alive probes: 60 seconds */
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 60L);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERPWD, RestClient::user_pass.c_str());
    /** set user agent */
    curl_easy_setopt(curl, CURLOPT_USERAGENT, RestClient::user_agent);
    /** set query URL */
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    /** Now specify we want to PUT data */
    curl_easy_setopt(curl, CURLOPT_PUT, 1L);
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    /** set read callback function */
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, RestClient::read_callback);
    /** set data object to pass to callback function */
    curl_easy_setopt(curl, CURLOPT_READDATA, &up_obj);
    /** set callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RestClient::write_callback);
    /** set data object to pass to callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ret);
    /** set the header callback function */
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, RestClient::header_callback);
    /** callback object for headers */
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &ret);
    /** set data size */
    curl_easy_setopt(curl, CURLOPT_INFILESIZE,
                     static_cast<long>(up_obj.length));

    /** set content-type header */
    curl_slist* header = NULL;
    header = curl_slist_append(header, ctype_header.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
    /** perform the actual query */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
      ret.body = "Failed to query.";
      ret.code = -1;
      return ret;
    }
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    ret.code = static_cast<int>(http_code);

    curl_slist_free_all(header);
    curl_easy_cleanup(curl);
  }
  return ret;
}
/**
 * @brief HTTP DELETE method
 *
 * @param url to query
 *
 * @return response struct
 */
RestClient::response RestClient::del(const std::string& url)
{
  /** create return struct */
  RestClient::response ret = {};

  /** we want HTTP DELETE */
  const char* http_delete = "DELETE";

  // use libcurl
  CURL *curl = NULL;
  CURLcode res = CURLE_OK;

  curl = GetCurlHandle();
  if (curl)
  {
      curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
      /* keep-alive idle time to 120 seconds */
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 1200L);
      /* interval time between keep-alive probes: 60 seconds */
      curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 60L);
    curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
    curl_easy_setopt(curl, CURLOPT_USERPWD, RestClient::user_pass.c_str());
    /** set user agent */
    curl_easy_setopt(curl, CURLOPT_USERAGENT, RestClient::user_agent);
    /** set query URL */
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    /** set HTTP DELETE METHOD */
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, http_delete);
    /** set callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, RestClient::write_callback);
    /** set data object to pass to callback function */
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ret);
    /** set the header callback function */
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, RestClient::header_callback);
    /** callback object for headers */
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, &ret);
    /** perform the actual query */
    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
      ret.body = "Failed to query.";
      ret.code = -1;
      return ret;
    }
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    ret.code = static_cast<int>(http_code);
    curl_easy_cleanup(curl);
  }
  return ret;
}

/**
 * @brief write callback function for libcurl
 *
 * @param data returned data of size (size*nmemb)
 * @param size size parameter
 * @param nmemb memblock parameter
 * @param userdata pointer to user data to save/work with return data
 *
 * @return (size * nmemb)
 */
size_t RestClient::write_callback(void *data, size_t size, size_t nmemb,
                            void *userdata)
{
  RestClient::response* r;
  r = reinterpret_cast<RestClient::response*>(userdata);
  r->body.append(reinterpret_cast<char*>(data), size*nmemb);

  return (size * nmemb);
}

/**
 * @brief header callback for libcurl
 *
 * @param data returned (header line)
 * @param size of data
 * @param nmemb memblock
 * @param userdata pointer to user data object to save headr data
 * @return size * nmemb;
 */
size_t RestClient::header_callback(void *data, size_t size, size_t nmemb,
                            void *userdata)
{
  RestClient::response* r;
  r = reinterpret_cast<RestClient::response*>(userdata);
  std::string header(reinterpret_cast<char*>(data), size*nmemb);
  size_t seperator = header.find_first_of(":");
  if ( std::string::npos == seperator ) {
    //roll with non seperated headers...
    trim(header);
    if ( 0 == header.length() ){
	return (size * nmemb); //blank line;
    }
    r->headers[header] = "present";
  } else {
    std::string key = header.substr(0, seperator);
    trim(key);
    std::string value = header.substr(seperator + 1);
    trim (value);
    r->headers[key] = value;
  }

  return (size * nmemb);
}

/**
 * @brief read callback function for libcurl
 *
 * @param pointer of max size (size*nmemb) to write data to
 * @param size size parameter
 * @param nmemb memblock parameter
 * @param userdata pointer to user data to read data from
 *
 * @return (size * nmemb)
 */
size_t RestClient::read_callback(void *data, size_t size, size_t nmemb,
                            void *userdata)
{
  /** get upload struct */
  RestClient::upload_object* u;
  u = reinterpret_cast<RestClient::upload_object*>(userdata);
  /** set correct sizes */
  size_t curl_size = size * nmemb;
  size_t copy_size = (u->length < curl_size) ? u->length : curl_size;
  /** copy data to buffer */
  memcpy(data, u->data, copy_size);
  /** decrement length and increment data pointer */
  u->length -= copy_size;
  u->data += copy_size;
  /** return copied size */
  return copy_size;
}
