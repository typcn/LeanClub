/*
 * Carmelo Porcelli
 */

#include <redis3m/redis3m.hpp>
#include <iostream>

using namespace redis3m;

int main (int argc, char **argv)
{
    connection::ptr_t conn = connection::create();
    conn->run(command("SUBSCRIBE") << "topic");
    while(true)
    {
        reply r = conn->get_reply();
        std::cout << "Received: " << r.elements().at(2).str() << std::endl;
    }
}
