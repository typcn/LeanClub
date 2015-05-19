// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#pragma once

#include <string>

namespace redis3m
{
namespace utils
{

std::string read_content_of_file(const std::string& path);

std::string datadir(const std::string& path="");

}
}
