#pragma once

#include <mutex>
#include <functional>
#include <forward_list>
#include <src/redis3m/include/redis3m/utils/noncopyable.h>

namespace redis3m
{
namespace utils
{

template<typename T>
class pool: noncopyable
{
    std::function<T(void)> builder;
    std::mutex access_mutex;
    std::forward_list<T> objects;

public:
    inline pool(const std::function<T(void)>& builder):
        builder(builder)
    {}

    T get()
    {
        T ret;
        access_mutex.lock();
        auto it = objects.begin();
        if (it != objects.end())
        {
            ret = *it;
            objects.erase(it);
        }
        access_mutex.unlock();
        if (!ret)
        {
            ret = builder();
        }
        return ret;
    }

    void put(const T& item)
    {
        std::unique_lock<std::mutex> lock(access_mutex);
        objects.push_front(item);
    }
};
}
}
