// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#include <redis3m/utils/file.h>
#include <fstream>

using namespace redis3m;

std::string utils::read_content_of_file(const std::string& path)
{
    std::ifstream ifs(path.c_str());
    return std::string( (std::istreambuf_iterator<char>(ifs) ),
                           (std::istreambuf_iterator<char>()    ) );
}
