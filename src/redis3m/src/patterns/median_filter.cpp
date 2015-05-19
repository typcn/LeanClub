// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details
#include <redis3m/patterns/median_filter.h>
#include <redis3m/utils/file.h>
#include <redis3m/command.h>

using namespace redis3m;
using namespace redis3m::patterns;

median_filter::median_filter(const std::string &prefix, int samples):
    _samples(samples)
{
    if (!prefix.empty())
    {
        _prefix= prefix+":";
    }
}

void median_filter::add_sample(connection::ptr_t connection, const std::string &tag, double value)
{
  connection->append(command("MULTI"));
  connection->append(command("LPUSH") << list_key(tag) << value );
  connection->append(command("LTRIM") << list_key(tag) << 0 << _samples-1 );
  connection->append(command("EXEC"));
  connection->get_replies(4);
}

double median_filter::median(connection::ptr_t connection, const std::string &tag)
{
    reply r = connection->run(command("SORT") << list_key(tag));
    const std::vector<reply>& values = r.elements();
    int size = (int)values.size();

    if (size % 2 != 0)
    {
		return std::stod(values.at((size - 1) / 2).str());
    }
    else
    {
        if (size == 0)
        {
            return 0;
        }
        else
        {
			return (std::stod(values.at(size / 2 - 1).str()) +
				std::stod(values.at(size / 2).str())) / 2;
        }
    }
}
