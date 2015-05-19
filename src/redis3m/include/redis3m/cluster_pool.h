#pragma once

#include <redis3m/utils/noncopyable.h>
#include <memory>
#include <string>
#include <vector>
#include <redis3m/reply.h>
#include <redis3m/connection.h>
#include <redis3m/utils/pool.h>
#include <map>
#include <mutex>
#include <set>
#include <forward_list>
#include <inttypes.h>

namespace redis3m
{
class cluster_pool: utils::noncopyable
{

public:
    REDIS3M_EXCEPTION(cannot_regenerate_slots_map)
    typedef std::shared_ptr<cluster_pool> ptr_t;

    inline static ptr_t create(const std::vector<std::pair<std::string, unsigned int>>& hosts)
    {
        return ptr_t(new cluster_pool(hosts));
    }

    reply run(const std::vector<std::string>& command,
              connection::role_t role = connection::MASTER,
              const std::string& slot="");

private:
    struct slot_interval
    {
        unsigned int begin;
        unsigned int end;
        inline bool operator< (const slot_interval& rvalue) const
        {
            return end < rvalue.begin;
        }
    };
    struct host_t
    {
        std::string address;
        unsigned int port;
        inline host_t():
            port(0)
        {

        }
        inline host_t(const std::pair<std::string, unsigned int>& pair):
            address(pair.first), port(pair.second)
        {
        }
        inline host_t(const std::string& address, unsigned int port):
            address(address), port(port)
        {

        }
        inline bool operator < (const host_t& rvalue) const
        {
            return (address <= rvalue.address) && ( port < rvalue.port);
        }
    };

    struct slot_hosts_t
    {
        host_t master;
        std::vector<host_t> slaves;
        inline slot_hosts_t(const host_t& master):
            master(master)
        {
        }
        inline slot_hosts_t(){}
    };

    void regenerate_slots_map();
    connection::ptr_t get_connection_for_host(const host_t& host);
    void put_connection_for_host(const host_t& host, connection::ptr_t conn);
    uint16_t slot_from_key(const std::string& key);

    cluster_pool(const std::vector<std::pair<std::string, unsigned int>>& hosts);
    std::mutex access_mutex;
    std::map<slot_interval, slot_hosts_t> slot_map;
    std::map<host_t, std::forward_list<connection::ptr_t>> connections_per_host;
    std::set<host_t> known_instances;
};
}
