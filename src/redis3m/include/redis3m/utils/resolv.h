// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#pragma once

#include <redis3m/utils/exception.h>
#include <string>
#include <vector>

namespace redis3m
{
namespace resolv
{
   REDIS3M_EXCEPTION(cannot_resolve_hostname)
   std::vector<std::string> get_addresses(const std::string &hostname);
}
}
