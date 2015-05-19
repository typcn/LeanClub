// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#include "common.h"

#include <boost/assign.hpp>
#include <thread>
#include <redis3m/utils/logging.h>
#include <boost/lexical_cast.hpp>
#include <functional>

using namespace redis3m;

void producer_f(connection_pool::ptr_t pool, const std::string& queue_name, const bool& do_work)
{
    while (do_work)
    {
        try
        {
            connection::ptr_t conn = pool->get();
            reply counter = conn->run(command("INCR")("counter"));
            conn->run(command("LPUSH")(queue_name)(boost::lexical_cast<std::string>(counter.integer())));
            pool->put(conn);
        }
        catch (const redis3m::transport_failure& ex)
        {
            logging::debug("Failure on producer");
            BOOST_FAIL("Failure on producer");
        }
        std::this_thread::yield();
    }
}

void consumer_f(connection_pool::ptr_t pool, const std::string& queue_name, const bool& do_work)
{
    while (do_work)
    {
        try
        {
            connection::ptr_t conn = pool->get();
            reply r = conn->run(command("BRPOP")(queue_name)("1"));
            pool->put(conn);
        }
        catch (const redis3m::transport_failure& ex)
        {
            logging::debug("Failure on consumer");
            BOOST_FAIL("Failure on consumer");
        }
        std::this_thread::yield();
    }
}

BOOST_AUTO_TEST_CASE( test_pool)
{
    connection_pool::ptr_t pool = connection_pool::create(std::string(getenv("REDIS_HOST")), "test");

    connection::ptr_t c;
    BOOST_CHECK_NO_THROW(c = pool->get(connection::MASTER));

    c->run(command("SET")("foo")("bar"));

    pool->put(c);

    BOOST_CHECK_NO_THROW(c = pool->get(connection::SLAVE));

    BOOST_CHECK_EQUAL(c->run(command("GET")("foo")).str(), "bar");
    BOOST_CHECK_THROW(c->run(command("SET")("foo")("bar")), slave_read_only);
}

BOOST_AUTO_TEST_CASE (run_with_connection)
{
    connection_pool::ptr_t pool = connection_pool::create(getenv("REDIS_HOST"), "test");

    pool->run_with_connection<void>([](connection::ptr_t c)
    {
       c->run(command("SET")("foo")("bar"));
       BOOST_CHECK_EQUAL(c->run(command("GET")("foo")).str(), "bar");
    });
}

BOOST_AUTO_TEST_CASE (crash_test)
{
    connection_pool::ptr_t pool = connection_pool::create(std::string(getenv("REDIS_HOST")), "test");

    bool do_work = true;
    std::vector<std::shared_ptr<std::thread>> producers;
    std::vector<std::shared_ptr<std::thread>> consumers;

    for (int i=0; i < 4; ++i)
    {
        producers.push_back( std::make_shared<std::thread>(std::bind(&producer_f, pool, "test-queue", std::ref(do_work))));
        consumers.push_back(std::make_shared<std::thread>(std::bind(&consumer_f, pool, "test-queue", std::ref(do_work))));
    }

    connection::ptr_t sentinel = connection::create(getenv("REDIS_HOST"), 26379);

    for (int i = 0; i < 5; ++i)
    {
        sentinel->run(command("SENTINEL") << "failover" << "test");
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    do_work=false;
    for (auto producer : producers)
    {
        producer->join();
    }
    for (auto consumer : consumers)
    {
        consumer->join();
    }
}

