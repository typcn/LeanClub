// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#pragma once

#include <string>
#include <set>
#include <redis3m/connection.h>
#include <memory>
#include <mutex>
#include <redis3m/utils/exception.h>
#include <redis3m/utils/noncopyable.h>

namespace redis3m {
    REDIS3M_EXCEPTION(cannot_find_sentinel)
    REDIS3M_EXCEPTION(cannot_find_master)
    REDIS3M_EXCEPTION(cannot_find_slave)
    REDIS3M_EXCEPTION(too_much_retries)
    REDIS3M_EXCEPTION(wrong_database)
    REDIS3M_EXCEPTION(role_dont_match)
    /**
     * @brief Manages a connection pool, using a Redis Sentinel
     * to get instances ip, managing also failover
     */
    class connection_pool: utils::noncopyable
    {
    public:
        typedef std::shared_ptr<connection_pool> ptr_t;

        /**
         * @brief Create a new connection_pool
         * @param sentinel_host Can be a single host or a list separate by commas,
         * if an host has multiple IPs, connection_pool tries all of them
         * @param master_name Master to lookup
         * @param sentinel_port Sentinel port, default 26379
         * @return
         */
        static inline ptr_t create(const std::string& sentinel_host,
                                   const std::string& master_name,
                                   unsigned int sentinel_port=26379)
        {
            return ptr_t(new connection_pool(sentinel_host, master_name, sentinel_port));
        }

        /**
         * @brief Ask for a connection
         * @param type Specify the type required, Master, Slave or Any
         * @return a valid connection object
         */
        connection::ptr_t get(connection::role_t type=connection::MASTER);

        /**
         * @brief Put a connection again on pool for reuse, pay attention to
         * insert only connection created from the same pool. Otherwise unpexpected
         * behaviours can happen.
         * @param conn
         */
        void put(connection::ptr_t conn );

        template<typename Ret>
        /**
         * @brief Execute a block of code passing a connection::ptr_t
         * if something fails, like broken connection, it will automatically
         * retry with an another one
         * @param f function to run, C++11 lambdas are perfect
         * @param conn_type type of connection required
         * @param retries how much retries do
         * @return
         */
        Ret run_with_connection(std::function<Ret(connection::ptr_t)> f,
                                connection::role_t conn_type = connection::MASTER,
                                unsigned int retries=5)
        {
            while (retries > 0)
            {
                try
                {
                    connection::ptr_t c = get(conn_type);
                    Ret r = f(c);
                    put(c);
                    return r;
                }
                catch (const connection_error& ex)
                {
                    --retries;
                }
            }
            throw too_much_retries();
        }

        /**
         * @brief Set a database to use on every new connection object created
         * by the pool.
         * @param value A valid database index
         */
        inline void set_database(unsigned int value) { _database = value; }

    private:
        connection_pool(const std::string& sentinel_host,
                        const std::string& master_name,
                        unsigned int sentinel_port);
        connection::ptr_t create_slave_connection();
        connection::ptr_t create_master_connection();
        connection::ptr_t sentinel_connection();
        static connection::role_t get_role(connection::ptr_t conn);
        std::mutex access_mutex;
        std::set<connection::ptr_t> connections;

        std::vector<std::string> sentinel_hosts;
        unsigned int sentinel_port;
        std::string master_name;
        unsigned int _database;
    };

    template<>
    void connection_pool::run_with_connection(std::function<void(connection::ptr_t)> f,
                                connection::role_t conn_type,
                                unsigned int retries);
}

