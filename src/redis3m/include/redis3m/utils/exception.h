// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#pragma once

#include <exception>
#include <string>

namespace redis3m {
    class exception: public std::exception
    {
    public:
        exception(const std::string& what):
        _what(what){}
        virtual ~exception() throw() {}
        
        inline virtual const char* what()
        {
            return _what.c_str();
        }
    private:
        std::string _what;
    };
}

#define REDIS3M_EXCEPTION(name) class name: public redis3m::exception {\
public: name(const std::string& what=""): exception(what){}};

#define REDIS3M_EXCEPTION_2(name, super) class name: public super {\
    public: name(const std::string& what=""): super(what){}};
