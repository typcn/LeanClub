// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#pragma once

#include <string>
#include <redis3m/utils/exception.h>
#include <redis3m/utils/noncopyable.h>
#include <redis3m/reply.h>
#include <vector>
#include <memory>

struct redisContext;

namespace redis3m {
REDIS3M_EXCEPTION(connection_error)
REDIS3M_EXCEPTION_2(unable_to_connect, connection_error)
REDIS3M_EXCEPTION_2(transport_failure, connection_error)
REDIS3M_EXCEPTION_2(slave_read_only, connection_error)

/**
* @brief The connection class, represent a connection to a Redis server
*/
class connection: utils::noncopyable
{
public:
    typedef std::shared_ptr<connection> ptr_t;

    /**
     * @brief Create and open a new connection
     * @param host hostname or ip of redis server, default localhost
     * @param port port of redis server, default: 6379
     * @return
     */
    inline static ptr_t create(const std::string& host="localhost",
                               const unsigned int port=6379)
    {
        return ptr_t(new connection(host, port));
    }

    ~connection();

    bool is_valid() const;

    /**
     * @brief Append a command to Redis server
     * @param args vector with args, example [ "SET", "foo", "bar" ]
     */
    void append(const std::vector<std::string>& args);

    /**
     * @brief Get a reply from server, blocking call if no reply is ready
     * @return reply object
     */
    reply get_reply();

    /**
     * @brief Get specific count of replies requested, blocking if they
     * are not ready yet.
     * @param count
     * @return
     */
    std::vector<reply> get_replies(unsigned int count);

    /**
     * @brief Utility to call append and then get_reply together
     * @param args same as {@link append()}
     * @return reply object
     */
    inline reply run(const std::vector<std::string>& args)
    {
        append(args);
        return get_reply();
    }

    /**
     * @brief Returns raw ptr to hiredis library connection.
     * Use it with caution and pay attention on memory
     * management.
     * @return
     */
    inline redisContext* c_ptr() { return c; }

    enum role_t {
        ANY = 0,
        MASTER = 1,
        SLAVE = 2
    };

private:
    friend class connection_pool;
    connection(const std::string& host, const unsigned int port);

    role_t _role;
    redisContext *c;
};
}
