#pragma once

namespace redis3m
{

namespace utils
{

class noncopyable
{
protected:
	noncopyable() = default;
	~noncopyable() = default;

	noncopyable( const noncopyable& ) = delete;
	noncopyable& operator=( const noncopyable& ) = delete;
};

}
}