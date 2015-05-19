#include <redis3m/redis3m.hpp>
#include <iostream>

using namespace redis3m;

int main(int argc, char **argv)
{
        connection::ptr_t conn = connection::create();
        conn->run(command("SET") << "foo" << "bar" );
        reply r = conn->run(command("GET") << "foo" );        
        std::cout << "FOO is: " << r.str() << std::endl;
}
