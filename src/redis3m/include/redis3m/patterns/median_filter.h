// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details
#pragma once

#include <redis3m/connection.h>

namespace redis3m
{
namespace patterns
{

/**
 * @brief Implement median filter algorithm on Redis.
 * A single instance can handle multiple filters, with a prefix.
 */
class median_filter
{
public:
    /**
     * @brief Constructor
     * @param prefix prefix to use for filters managed by this instance
     * @param samples numbers of samples
     */
    median_filter(const std::string& prefix="", int samples=11);

    /**
     * @brief Adds a sample to a filter
     * @param connection
     * @param tag identifies a specific filter
     * @param value sample value
     */
    void add_sample(connection::ptr_t connection, const std::string& tag, double value);

    /**
     * @brief Get actual median from samples stored on db
     * @param connection
     * @param tag identifies a specific filter
     * @return median value
     */
    double median(connection::ptr_t connection, const std::string& tag);

    /**
     * @brief List used to store samples in arrival order.
     * Use this method to do something on this key, like set an expiring.
     * @param tag identifies a specific filter
     * @return
     */
    inline std::string list_key(const std::string& tag)
    {
         return _prefix + tag;
    }

private:
    std::string _prefix;
    int _samples;
};
}
}
