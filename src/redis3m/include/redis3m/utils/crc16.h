#pragma once

#include <inttypes.h>
#include <string>

namespace redis3m
{
namespace utils
{

uint16_t crc16(const std::string& string);

}
}
