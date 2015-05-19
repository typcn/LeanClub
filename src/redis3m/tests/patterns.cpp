// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#include "common.h"

#include <redis3m/patterns/patterns.hpp>

#include <boost/assign.hpp>

using namespace redis3m;

class test_model: public redis3m::patterns::model
{
public:
    test_model():
        model(){}

    test_model(const std::string& id, const std::map<std::string, std::string>& map):
        model(id, map),
        _field(read_str_from_map(map, "field")),
        _field2(read_str_from_map(map, "field2"))
    {}

    test_model(const std::string& id, const std::string& field1):
        model(),
        _field(field1)
    {
        _id = id;
        _loaded = true;
    }

    std::map<std::string, std::string> to_map() const
    {
        return boost::assign::map_list_of("field", _field);
    }

    static std::string model_name()
    {
        return "Test";
    }

    static std::vector<std::string> indices()
    {
        return boost::assign::list_of("field");
    }

    static std::vector<std::string> tracked()
    {
        return boost::assign::list_of("mylist");
    }

    void set_field(const std::string& value) { _field = value; }

    REDIS3M_MODEL_RO_ATTRIBUTE(std::string, field)
    REDIS3M_MODEL_RO_ATTRIBUTE(std::string, field2)
};

BOOST_AUTO_TEST_CASE ( script_exec )
{
    test_connection tc;

    patterns::script_exec ping_script("return redis.call(\"PING\")");

    reply r = ping_script.exec(*tc);
    BOOST_CHECK_EQUAL(r.str(), "PONG");
}

BOOST_AUTO_TEST_CASE ( scheduler_test )
{
    test_connection tc;
    patterns::scheduler s("test-queue");

    // Enqueue
    s.enqueue(*tc, "testid", boost::posix_time::seconds(1));

    // Not expired yet
    BOOST_CHECK_EQUAL( s.find_expired(*tc), "");
    sleep(2);

    // Now it should expire
    std::string found_id = s.find_expired(*tc, boost::posix_time::seconds(1));
    BOOST_CHECK_EQUAL( found_id, "testid");
    sleep(2);

    // Expire again after lock_for passed
    found_id = s.find_expired(*tc, boost::posix_time::seconds(2));
    BOOST_CHECK_EQUAL( found_id, "testid");

    // Dequeue
    s.dequeue(*tc, found_id);

    reply r = tc->run(command("ZCARD")("test-queue"));
    BOOST_CHECK_EQUAL(r.integer(), 0);
}

BOOST_AUTO_TEST_CASE ( simple_obj_store_test )
{
    test_connection tc;

    patterns::simple_obj_store<test_model> store;

    test_model new_m("xxx", "test");

    store.save(*tc, new_m);

    test_model restored = store.find(*tc, "xxx");

    BOOST_CHECK_EQUAL(restored.loaded(), true);

    BOOST_CHECK_EQUAL(restored.field(), "test");

    store.remove(*tc, restored);

    test_model restored2 = store.find(*tc, "xxx");
    BOOST_CHECK_EQUAL(restored2.loaded(), false);
}


BOOST_AUTO_TEST_CASE ( orm_save_test )
{
    test_connection tc;

    patterns::orm<test_model> store;

    test_model new_m("", "test");

    std::string id = store.save(*tc, new_m);

    test_model restored = store.find_by_id(*tc, id);
    BOOST_CHECK_EQUAL(restored.loaded(), true);
    BOOST_CHECK_EQUAL(restored.field(), "test");

    store.remove(*tc, restored);

    test_model restored2 = store.find_by_id(*tc, id);
    BOOST_CHECK_EQUAL(restored2.loaded(), false);
}

BOOST_AUTO_TEST_CASE ( orm_tracked_key )
{
    test_connection tc;

    patterns::orm<test_model> store;

    test_model new_m;
    new_m.set_field("xxx");

    std::string id = store.save(*tc, new_m);

    BOOST_CHECK_NE(id, "");

    tc->run(command("LPUSH")(store.tracked_key(id, "mylist"))("yyy"));

    test_model restored = store.find_by_id(*tc, id);
    store.remove(*tc, restored);

    BOOST_CHECK_EQUAL(tc->run(command("EXISTS")(store.tracked_key(id, "mylist"))).integer(), 0);
}

BOOST_AUTO_TEST_CASE( median )
{
    test_connection tc;
    patterns::median_filter filter;

    filter.add_sample(*tc, "even", 1);
    filter.add_sample(*tc, "even", 0);
    filter.add_sample(*tc, "even", 4);
    filter.add_sample(*tc, "even", 5);
    filter.add_sample(*tc, "even", 8);
    filter.add_sample(*tc, "even", 10);

    BOOST_CHECK_CLOSE(filter.median(*tc, "even"), 4.5, 0.1);

    filter.add_sample(*tc, "odd", 1);
    filter.add_sample(*tc, "odd", 0);
    filter.add_sample(*tc, "odd", 3.5);
    filter.add_sample(*tc, "odd", 5);
    filter.add_sample(*tc, "odd", 10);

    BOOST_CHECK_CLOSE(filter.median(*tc, "odd"), 3.5, 0.1);

    // Over samples

    filter.add_sample(*tc, "over", 90);
    filter.add_sample(*tc, "over", 1);
    filter.add_sample(*tc, "over", 0);
    filter.add_sample(*tc, "over", 3.5);

    filter.add_sample(*tc, "over", 5);
    filter.add_sample(*tc, "over", 10);

    filter.add_sample(*tc, "over", 80);
    filter.add_sample(*tc, "over", 1);
    filter.add_sample(*tc, "over", 28);

    filter.add_sample(*tc, "over", 15);
    filter.add_sample(*tc, "over", 35);
    filter.add_sample(*tc, "over", 47);

    filter.add_sample(*tc, "over", 100);
    filter.add_sample(*tc, "over", 19);
    filter.add_sample(*tc, "over", 31);

    BOOST_CHECK_CLOSE(filter.median(*tc, "over"), 28, 0.1);

    BOOST_CHECK_CLOSE(filter.median(*tc, "not_exist"), 0, 0.1);

    filter.add_sample(*tc, "one", 3);
    BOOST_CHECK_CLOSE(filter.median(*tc, "one"), 3, 0.1);
}
