// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#pragma once

#include <redis3m/reply.h>
#include <redis3m/connection.h>
#include <vector>
#include <string>

namespace redis3m {
namespace patterns
{

/**
 * @brief Helps to run Lua scripts on a Redis instance.
 * It will take care to use EVALSHA to optimize performance and then
 * EVAL if the script is not yet available on Redis server.
 * See http://redis.io/commands/eval for other infos.
 */
class script_exec {
public:
    /**
     * @brief Create script_exec
     * @param script script content or a path to a file which contains the script
     * @param is_path true if previous argument is a path, false otherwise. The latter
     * is default.
     */
    script_exec(const std::string& script, bool is_path=false);

    /**
     * @brief Execute the script. First trying with EVALSHA, then with EVAL
     * @param connection
     * @param keys vector of keys used by the script
     * @param args
     * @return
     */
    reply exec(connection::ptr_t connection,
               const std::vector<std::string>& keys=std::vector<std::string>(),
               const std::vector<std::string>& args=std::vector<std::string>());
private:
    std::string _script;
    bool _is_path;
    std::string _sha1;
};
}
}
