// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details
#pragma once

#define BOOST_TEST_MODULE redis3m
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <redis3m/redis3m.hpp>

namespace redis3m
{
class test_connection
{
public:
    test_connection()
    {
        char * host_cptr = getenv("REDIS_HOST");
        if (host_cptr)
        {
            c = redis3m::connection::create(host_cptr);
        }
        else
        {
            c = redis3m::connection::create();
        }
        c->run(command("FLUSHDB"));
    }

    inline redis3m::connection::ptr_t operator*()
    {
        return c;
    }

    inline redis3m::connection::ptr_t operator->()
    {
        return c;
    }

    redis3m::connection::ptr_t c;
};

class test_simple_pool
{
public:
    test_simple_pool()
    {
        char * host_cptr = getenv("REDIS_HOST");
        if (host_cptr)
        {
            sp = redis3m::simple_pool::create(host_cptr);
        }
        else
        {
            sp = redis3m::simple_pool::create();
        }
        redis3m::connection::ptr_t conn = sp->get();
        conn->run(command("FLUSHDB"));
        sp->put(conn);
    }

    inline redis3m::simple_pool::ptr_t operator*()
    {
        return sp;
    }

    inline redis3m::simple_pool::ptr_t operator->()
    {
        return sp;
    }

    redis3m::simple_pool::ptr_t sp;
};
}
