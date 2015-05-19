// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#include <redis3m/utils/logging.h>
#include <iostream>

using namespace redis3m;

logging::ptr_t logging::logger(new logging());

void logging::debug_impl(const std::string &string)
{
    std::unique_lock<std::mutex> lock(access);
    std::cerr << string << std::endl;
}

void logging::warning_impl(const std::string &string)
{
    std::unique_lock<std::mutex> lock(access);
    std::cerr << string << std::endl;
}

void logging::error_impl(const std::string &string)
{
    std::unique_lock<std::mutex> lock(access);
    std::cerr << string << std::endl;
}

