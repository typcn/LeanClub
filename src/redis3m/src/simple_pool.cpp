// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#include <redis3m/simple_pool.h>
#include <redis3m/command.h>

using namespace redis3m;

connection::ptr_t simple_pool::get()
{
    connection::ptr_t ret;

    access_mutex.lock();
    std::set<connection::ptr_t>::iterator it = connections.begin();
    if (it != connections.end())
    {
        ret = *it;
        connections.erase(it);
    }

    access_mutex.unlock();

    if (!ret)
    {
        ret = connection::create(_host, _port);
        // Setup connections selecting db
        if (_database != 0)
        {
            ret->run(command("SELECT") << _database);
        }
    }
    return ret;
}


void simple_pool::put(connection::ptr_t conn)
{
    if (conn->is_valid())
    {
        std::unique_lock<std::mutex> lock(access_mutex);
        connections.insert(conn);
    }
}

simple_pool::simple_pool(const std::string &host, unsigned int port):
    _host(host),
    _port(port),
    _database(0)
{

}

template<>
void simple_pool::run_with_connection(std::function<void(connection::ptr_t)> f,
                                unsigned int retries)
{
    while (retries > 0)
    {
        try
        {
            connection::ptr_t c = get();
            f(c);
            put(c);
            return;
        } catch (const connection_error& )
        {
            --retries;
        }
    }
    throw too_much_retries();
}
