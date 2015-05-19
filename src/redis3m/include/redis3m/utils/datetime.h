// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#pragma once

#include <stdint.h>
#include <boost/date_time/posix_time/ptime.hpp>

namespace redis3m
{
namespace datetime
{

uint64_t utc_now_in_seconds();
uint64_t ptime_in_seconds(const boost::posix_time::ptime& time);
boost::posix_time::ptime now();

}
}
