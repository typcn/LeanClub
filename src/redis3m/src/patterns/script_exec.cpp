// Copyright (c) 2014 Luca Marturana. All rights reserved.
// Licensed under Apache 2.0, see LICENSE for details

#include <redis3m/patterns/script_exec.h>
#include <redis3m/utils/sha1.h>
#include <redis3m/utils/file.h>


using namespace redis3m;

patterns::script_exec::script_exec(const std::string& script, bool is_path):
_script(script),
_is_path(is_path)
{
    unsigned char hash[20];
    char hexstring[41];
    if (_is_path)
    {
        std::string script_content = utils::read_content_of_file(script);
        sha1::calc(script_content.c_str(),(int)script_content.size(),hash);
    }
    else
    {
        sha1::calc(script.c_str(),(int)script.size(),hash);
    }
    sha1::toHexString(hash, hexstring);
    _sha1.assign(hexstring);
}

reply patterns::script_exec::exec(connection::ptr_t connection,
                const std::vector<std::string>& keys,
                const std::vector<std::string>& args)
{
    std::vector<std::string> exec_command;
    exec_command.reserve(3+keys.size()+args.size());

    exec_command.push_back("EVALSHA");
    exec_command.push_back(_sha1);
	exec_command.push_back(std::to_string(keys.size()));
    exec_command.insert(exec_command.end(), keys.begin(), keys.end());
    exec_command.insert(exec_command.end(), args.begin(), args.end());
    reply r = connection->run(exec_command);
    if (r.type() == reply::type_t::ERROR &&
		r.str().find("NOSCRIPT") == 0
		)
    {
        exec_command[0] = "EVAL";
        if (_is_path)
        {
            exec_command[1] = utils::read_content_of_file(_script);
        }
        else
        {
            exec_command[1] = _script;
        }
        r = connection->run(exec_command);
    }
    return r;
}
