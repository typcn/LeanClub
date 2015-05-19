#include <redis3m/connection.h>
#include <redis3m/patterns/scheduler.h>
#include <iostream>

using namespace redis3m;

int main(int argc, char **argv)
{
  connection::ptr_t conn = connection::create();
  
  patterns::script_exec ping_script("return redis.call(\"PING\")");

  // Automatic SHA1 calculation, first tries with EVALSHA then with EVAL
  reply r = ping_script.exec(conn);
  std::cout << r.str() << std::endl;
}
