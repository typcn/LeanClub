// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#include <hiredis/hiredis.h>
#include <redis3m/reply.h>

using namespace redis3m;

reply::reply(redisReply *c_reply):
_type(type_t::ERROR),
_integer(0)
{
    _type = static_cast<type_t>(c_reply->type);
    switch (_type) {
	case type_t::ERROR:
	case type_t::STRING:
	case type_t::STATUS:
            _str = std::string(c_reply->str, c_reply->len);
            break;
	case type_t::INTEGER:
            _integer = c_reply->integer;
            break;
	case type_t::ARRAY:
            for (size_t i=0; i < c_reply->elements; ++i) {
                _elements.push_back(reply(c_reply->element[i]));
            }
            break;
        default:
            break;
    }
}
