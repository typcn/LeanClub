// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#include <redis3m/patterns/scheduler.h>
#include <redis3m/utils/datetime.h>
#include <boost/lexical_cast.hpp>
#include <redis3m/utils/file.h>
#include <boost/assign/list_of.hpp>
#include <redis3m/command.h>

using namespace redis3m;
using namespace redis3m::patterns;

script_exec scheduler::find_expired_script(utils::datadir("/lua/scheduler.lua"), true);

scheduler::scheduler(const std::string &queue_name):
    _queue(queue_name)
{

}

void scheduler::append_enqueue(connection::ptr_t connection, const std::string &object_id,
                               const boost::posix_time::ptime& time)
{
    connection->append(command("ZADD")(_queue)
                            (boost::lexical_cast<std::string>(datetime::ptime_in_seconds(time)))
                            (object_id));
}

void scheduler::append_enqueue(connection::ptr_t connection, const std::string &object_id,
                               const boost::posix_time::time_duration& delay)
{
    append_enqueue(connection, object_id, datetime::now()+delay);
}

void scheduler::enqueue(connection::ptr_t connection, const std::string &object_id,
                        const boost::posix_time::ptime& time)
{
    append_enqueue(connection, object_id, time);
    connection->get_reply();
}

void scheduler::enqueue(connection::ptr_t connection, const std::string &object_id,
                               const boost::posix_time::time_duration& delay)
{
    append_enqueue(connection, object_id, delay);
    connection->get_reply();
}

void scheduler::append_dequeue(connection::ptr_t connection, const std::string &object_id)
{
    connection->append(command("ZREM")(_queue)(object_id));
}

void scheduler::dequeue(connection::ptr_t connection, const std::string &object_id)
{
    append_dequeue(connection, object_id);
    connection->get_reply();
}

std::string scheduler::find_expired(connection::ptr_t connection, const boost::posix_time::time_duration& lock_for)
{
    boost::posix_time::ptime now = datetime::now();
    std::string now_s = boost::lexical_cast<std::string>(datetime::ptime_in_seconds(now));
    std::string now_and_lock_for = boost::lexical_cast<std::string>(datetime::ptime_in_seconds(now+lock_for));
    reply r = find_expired_script.exec(connection,
                                boost::assign::list_of(_queue),
                                boost::assign::list_of(now_s)(now_and_lock_for));
    std::string ret;
    if (r.type() == reply::type_t::STRING)
    {
        ret = r.str();
    }
    return ret;
}

