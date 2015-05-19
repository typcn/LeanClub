/*
 * Carmelo Porcelli
 */

#include <redis3m/redis3m.hpp>
#include <iostream>

using namespace redis3m;

int main(int argc, char **argv)
{
        connection::ptr_t conn = connection::create();
        std::string body;
        if (argv[1] != NULL)
        {
            body = argv[1];
        }
        else
        {
            body = "Undefined";
        }
        reply r = conn->run(command("PUBLISH") << "topic" << body);
}
