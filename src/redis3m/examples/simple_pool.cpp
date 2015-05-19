#include <redis3m/redis3m.hpp>
#include <iostream>

using namespace redis3m;

int main(int argc, char **argv)
{
    simple_pool::ptr_t pool = simple_pool::create("yourhost");
    
    connection::ptr_t c = pool->get();
    c->run(command("SET")("foo")("bar"));
    pool->put(c);

    c = pool->get();
    std::cout << c->run(command("GET")("foo")).str();
    pool->put(c);

    /* If you have C++11
      // Automatically retries on connection failures
      pool->run_with_connection<void>([&](connection::ptr_t conn)
      {
       conn->run(command("SET")("foo")("bar"));
      });
    */
}
