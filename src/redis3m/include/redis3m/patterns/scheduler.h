// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#pragma once

#include <string>
#include <redis3m/patterns/script_exec.h>
#include <redis3m/connection.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

namespace redis3m
{
namespace patterns
{

/**
 * @brief A scheduler pattern, can be useful to manage "jobs" that needs to be run
 * at a given time. It's fault tolerant and scalable. Multiple workers can be dispatched
 * and jobs will be executed only once.
 * See http://luca3m.me/2013/12/03/redis-scheduler.html for other infos.
 */
class scheduler
{
public:
    /**
     * @brief Scheduler constructor
     * @param queue_name redis key used to store jobs, in a ZSET
     */
    scheduler(const std::string& queue_name);

    /**
     * @brief Append Redis commands needed to enqueue a job, useful to use inside transactions
     * @param connection a valid redis connection
     * @param object_id an object id, can be everything, like a reference to an Redis hash
     * @param time absolute UTC time, when job will fire
     */
    void append_enqueue(connection::ptr_t connection, const std::string& object_id, const boost::posix_time::ptime& time);

    /**
     * @brief Append Redis commands needed to enqueue a job, useful to use inside transactions
     * @param connection a valid redis connection
     * @param object_id an object id, can be everything, like a reference to an Redis hash
     * @param delay Job will fire at now() + delay
     */
    void append_enqueue(connection::ptr_t connection, const std::string& object_id, const boost::posix_time::time_duration& delay);

    /**
     * @brief Put a job in schedule
     * @param connection a valid redis connection
     * @param object_id an object id, can be everything, like a reference to an Redis hash
     * @param time absolute UTC time, when job will fire
     */
    void enqueue(connection::ptr_t connection, const std::string& object_id, const boost::posix_time::ptime& time);

    /**
     * @brief Put a job in schedule
     * @param connection a valid redis connection
     * @param object_id an object id, can be everything, like a reference to an Redis hash
     * @param delay Job will fire at now() + delay
     */
    void enqueue(connection::ptr_t connection, const std::string& object_id, const boost::posix_time::time_duration& delay);

    /**
     * @brief Append Redis commands to remove a job from queue, use it when worker ends a job. Otherwise it will
     * fire on another worker. It's like {@link dequeue()} but to be used inside transactions.
     * @param connection
     * @param object_id
     */
    void append_dequeue(connection::ptr_t connection, const std::string& object_id);

    /**
     * @brief Remove a job from queue, use it when worker ends a job. Otherwise it will
     * fire on another worker
     * @param connection
     * @param object_id
     */
    void dequeue(connection::ptr_t connection, const std::string& object_id);

    /**
     * @brief Return a fired job, it will be locked for an amount of time. During this time
     * the worker will run the job and then call {@link dequeue()}. If worker crashes,
     * the job will fire on another worker after lock period.
     * @param connection
     * @param lock_for lock delay, by default 1 min
     * @return An object id or an empty string if none found
     */
    std::string find_expired(connection::ptr_t connection, const boost::posix_time::time_duration& lock_for=boost::posix_time::seconds(60));

private:
    std::string _queue;
    static script_exec find_expired_script;
};
}
}
