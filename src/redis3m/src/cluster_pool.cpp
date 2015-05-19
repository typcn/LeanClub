#include <redis3m/cluster_pool.h>
#include <redis3m/utils/logging.h>
#include <redis3m/utils/crc16.h>
#include <redis3m/command.h>
#ifndef NO_BOOST
#include <boost/format.hpp>
#endif
#include <regex>
#include <assert.h>

using namespace redis3m;

reply cluster_pool::run(const std::vector<std::string> &command,
                        redis3m::connection::role_t /*role*/,
                        const std::string &key_slot)
{
    uint16_t slot=0x4000;
    if (key_slot.empty())
    {
        if (command.size() > 1)
        {
            slot = slot_from_key(command.at(1));
        }
    }
    else
    {
        slot = slot_from_key(key_slot);
    }

    host_t host;
    if (slot < 0x4000)
    {
        auto slot_map_it = std::find_if(slot_map.begin(), slot_map.end(),
                   [&](const std::pair<slot_interval, slot_hosts_t>& item )
        {
            const slot_interval& interval = item.first;
            return interval.begin <= slot && slot <= interval.end;
        });
        assert(slot_map_it != slot_map.end());
        const slot_hosts_t& slot_hosts = slot_map_it->second;
        host = slot_hosts.master;
    }
    else
    {
        // TODO: choose a random node
        host = *known_instances.begin();
    }
    connection::ptr_t conn = get_connection_for_host(host);

    reply r = conn->run(command);
    put_connection_for_host(host, conn);
    return r;
}

void cluster_pool::regenerate_slots_map()
{
    slot_map.clear();
    for ( auto host : known_instances)
    {
        try
        {
            connection::ptr_t conn = get_connection_for_host(host);
            reply r = conn->run(command("CLUSTER") << "SLOTS");
            put_connection_for_host(host, conn);
            /* Example output:
            1) 1) (integer) 0
               2) (integer) 5460
               3) 1) "192.168.57.2"
                  2) (integer) 7000
               4) 1) "192.168.57.3"
                  2) (integer) 7001
            2) 1) (integer) 5461
               2) (integer) 10922
               3) 1) "192.168.57.3"
                  2) (integer) 7000
               4) 1) "192.168.57.2"
                  2) (integer) 7001
            3) 1) (integer) 10923
               2) (integer) 16383
               3) 1) "192.168.57.4"
                  2) (integer) 7000
               4) 1) "192.168.57.4"
                  2) (integer) 7001
                  */
            for (auto slot : r.elements())
            {
                auto slot_it = slot.elements().begin();
                slot_interval interval;
                interval.begin = (int)slot_it->integer();
                ++slot_it;
                interval.end = (int)slot_it->integer();
                ++slot_it;
                slot_hosts_t hosts(host_t(slot_it->elements().at(0), (int)slot_it->elements().at(1)));
                ++slot_it;
                for ( ; slot_it != slot.elements().end(); ++slot_it)
                {
                    hosts.slaves.push_back(host_t(slot_it->elements().at(0), (int)slot_it->elements().at(1)));
                }
                slot_map[interval] = hosts;
            }
            return;
        }
        catch ( const connection_error&)
        {
#ifndef NO_BOOST
            logging::warning(boost::str(boost::format("Host %s:%p is down, trying with another one") % host.address % host.port));
#endif
        }
    }
    throw cannot_regenerate_slots_map();
}

connection::ptr_t cluster_pool::get_connection_for_host(const cluster_pool::host_t &host)
{
    std::forward_list<connection::ptr_t>& pool = connections_per_host[host];
    connection::ptr_t ret;

    if ( ! pool.empty() )
    {
        ret = pool.front();
        pool.pop_front();
    }
    else
    {
        ret = connection::create(host.address, host.port);
    }
    return ret;
}

void cluster_pool::put_connection_for_host(const cluster_pool::host_t & /*host*/, connection::ptr_t /*conn*/)
{
    // TODO: put connection on pool for reuse
}

uint16_t cluster_pool::slot_from_key(const std::string &key)
{
    static const std::regex slot_regex("[^{]*\\{([^}]+)\\}.*");
    std::smatch results;
    uint16_t slot = 0;
    if ( std::regex_match(key, results, slot_regex))
    {
        slot = utils::crc16(results[1]);
    }
    else
    {
        slot = utils::crc16(key);
    }
    return slot & 0x3FFF;
}

cluster_pool::cluster_pool(const std::vector<std::pair<std::string, unsigned int> > &hosts)
{
    for (auto item: hosts)
    {
        known_instances.insert(host_t(item.first, item.second));
    }
    regenerate_slots_map();
}
