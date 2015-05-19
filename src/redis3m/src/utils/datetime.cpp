// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#include <redis3m/utils/datetime.h>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace redis3m;

uint64_t datetime::utc_now_in_seconds()
{
    boost::posix_time::ptime now = boost::posix_time::second_clock::universal_time();
    return ptime_in_seconds(now);
}

uint64_t datetime::ptime_in_seconds(const boost::posix_time::ptime &time)
{
    static const boost::posix_time::ptime epoch(boost::gregorian::date(1970,1,1));
    boost::posix_time::time_duration seconds_since_epoch = time - epoch;
    return seconds_since_epoch.total_seconds();
}

boost::posix_time::ptime datetime::now()
{
    return boost::posix_time::second_clock::universal_time();
}
