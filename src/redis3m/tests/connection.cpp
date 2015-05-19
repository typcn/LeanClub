// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#include "common.h"

using namespace redis3m;

BOOST_AUTO_TEST_CASE ( fail_connect )
{
    BOOST_CHECK_THROW(connection::create("localhost", 9090), unable_to_connect);
}

BOOST_AUTO_TEST_CASE( correct_connection )
{
    BOOST_CHECK_NO_THROW(test_connection());
}

// hiredis bug
//BOOST_AUTO_TEST_CASE( ipv6_connection )
//{
//    BOOST_CHECK_NO_THROW(connection::create("::1", 6379));
//}

BOOST_AUTO_TEST_CASE( test_info)
{
    test_connection tc;
    redis3m::reply r = tc->run(command("INFO"));
}

BOOST_AUTO_TEST_CASE( test_ping)
{
    test_connection tc;
    reply r = tc->run(command("PING"));
    BOOST_CHECK_EQUAL(r.str(), "PONG");
}

BOOST_AUTO_TEST_CASE( test_reply_operators)
{
    test_connection tc;
    reply r = tc->run(command("PING"));
    std::string value = r;
    BOOST_CHECK_EQUAL(value, "PONG");
    BOOST_CHECK_EQUAL(r, "PONG");
}

BOOST_AUTO_TEST_CASE( set_get)
{
    test_connection tc;

    BOOST_CHECK_EQUAL("", tc->run(command("GET") << "foo" ).str());
    BOOST_CHECK_NO_THROW(tc->run(command("SET") << "foo" << "bar"));
    BOOST_CHECK_EQUAL("bar", tc->run(command("GET") << "foo" ).str());
}

BOOST_AUTO_TEST_CASE( test_types)
{
    test_connection tc;

    tc->run(command("SET") << "double" << 0.40);

    BOOST_CHECK_CLOSE(std::stod(tc->run(command("GET") << "double").str()), 0.40, 0.1);

    tc->run(command("SET") << "test" << 100);
    BOOST_CHECK_EQUAL(std::stoi(tc->run(command("GET") << "test").str()), 100);

    tc->run(command("SET") << "test" << "xxx");
    BOOST_CHECK_EQUAL(tc->run(command("GET") << "test").str(), std::string("xxx"));
}

BOOST_AUTO_TEST_CASE( test_pool)
{
    test_simple_pool pool;

    connection::ptr_t c = pool->get();

    c->run(command("SET")("foo")("bar"));

    pool->put(c);

    c = pool->get();

    BOOST_CHECK_EQUAL(c->run(command("GET")("foo")).str(), "bar");
    pool->put(c);
}
