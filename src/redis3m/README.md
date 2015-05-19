redis3m
=======
[![Build Status](https://travis-ci.org/luca3m/redis3m.png?branch=master)](https://travis-ci.org/luca3m/redis3m)

A C++ [Redis](http://redis.io) client, born to bring my experience using Redis and C++ on a opensource library.

### Main goals

1. Provide a simple and efficient wrapper of [hiredis](http://github.com/redis/hiredis), with C++ facilities like memory management
2. A connection pooling system, with support for high availability using sentinel
3. A set of useful patterns ready to use and composable with other code. For example [scheduler](http://luca3m.me/2013/12/03/redis-scheduler.html), [orm](http://github.com/soveran/ohm), counters or message queueing

### Dependencies

redis3m requires hiredis and boost libraries.

### Install

First step install all required dependencies, on a Debian system you can use:

```bash
sudo apt-get install libmsgpack-dev libboost-thread-dev libboost-date-time-dev libboost-test-dev libboost-filesystem-dev libboost-system-dev libhiredis-dev cmake build-essential
```

Then checkout the code and compile it
```bash
git clone https://github.com/luca3m/redis3m
cd redis3m
cmake
make
sudo make install
```

### Documentation

See [examples](https://github.com/luca3m/redis3m/tree/master/examples) directory for some examples, you can compile them with:

```bash
g++ <example.cpp> $(pkg-config --cflags --libs redis3m) -o <example.bin>
```

You can find all classes reference [here](http://luca3m.me/redis3m/docs)

### Versioning

This project uses [semantic versioning](http://semver.org). In short words versions are named X.Y[.Z].
Changing X means break API changes, Y means new features without breaking old code, Z means bug fixing.
