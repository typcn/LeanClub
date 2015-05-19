#include "common.h"
#include <boost/assign/list_of.hpp>

using namespace redis3m;

cluster_pool::ptr_t get_pool()
{
    return redis3m::cluster_pool::create(
                boost::assign::list_of(
                    std::make_pair("redis01", 7000)
                )
            );
}

BOOST_AUTO_TEST_CASE( create)
{
    BOOST_CHECK_NO_THROW(get_pool());
}

BOOST_AUTO_TEST_CASE( set_key)
{
    redis3m::cluster_pool::ptr_t pool = get_pool();
    BOOST_CHECK_EQUAL("OK", pool->run(command("SET")("foo")("bar")));
    BOOST_CHECK_EQUAL("bar", pool->run(command("GET")("foo")));
    //BOOST_CHECK_EQUAL(reply::type_t::NIL, pool->run(command("GET")("test")).type());
}
