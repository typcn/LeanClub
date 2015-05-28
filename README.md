# LeanClub
High performance C++ forum system

WARNING: Not recommended for production

DEMO: https://leanclub.org/

# Features
1. Extremely Fast , 20000 RPS on i7 iMac.
2. Simple and clean frontend.
3. Full ajax and API.
4. Google GitHub Twitter OAuth sign in.

# Minimum Requirements
- GCC 4.8
- CMake 2.8 

(Windows is not supported)

# Setup
### Datebase
1. Install [Apache Couchdb](https://couchdb.apache.org/)
2. Create 4 databases named “category” “notification” “topics” and “user”.
3. Import design docs from “databases” directory.

### Environment
1. ```apt-get install build-essential cmake libcurl4-nss-dev libboost-all-dev redis-server libtcmalloc-minimal4 && sudo ln -s /usr/lib/libtcmalloc_minimal.so.4 /usr/lib/libtcmalloc_minimal.so```
2. Install [hiredis](https://github.com/redis/hiredis) and [redis3m](https://github.com/luca3m/redis3m) from sources

### Config
	cp src/config.h.example.h src/config.h
	vi src/config.h

### Build and run
	mkdir Build
	cd Build
	cmake ..
	make
	./leanclub
# About
This program based on: [Crow](https://github.com/ipkn/crow)
I’m newbie for c++ , the code may “buggy" , not recommended for production.

### License
MIT


