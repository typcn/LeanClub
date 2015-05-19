#include <redis3m/redis3m.hpp>
#include <iostream>

using namespace redis3m;

int main(int argc, char **argv)
{
    connection_pool::ptr_t pool = connection_pool::create("<yoursentinel>", "test");
    
    connection::ptr_t c = pool->get(connection::MASTER);
    c->run(command("SET") << "foo" << "bar");
    pool->put(c);

    c = pool->get(connection::SLAVE);
    std::cout << c->run(command("GET") << "foo" ).str();
    pool->put(c);
}
