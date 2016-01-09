# LeanClub
[![Build Status](https://travis-ci.org/typcn/LeanClub.svg?branch=master)](https://travis-ci.org/typcn/LeanClub)

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
3. Import design docs from “[databases](https://github.com/typcn/LeanClub/tree/master/databases)” directory.

### Environment
1. ```apt-get install build-essential cmake libcurl4-nss-dev libboost-all-dev redis-server libhiredis-dev libtcmalloc-minimal4 && sudo ln -s /usr/lib/libtcmalloc_minimal.so.4 /usr/lib/libtcmalloc_minimal.so```
2. Install [hiredis](https://github.com/redis/hiredis) from sources ( redis3m is already included )

### Config
	cp src/config.h.example.h src/config.h
	vi src/config.h

### Build and run
	mkdir Build
	cd Build
	cmake ..
	make
	./leanclub
### Nginx

    server {
        listen 80;
        server_name leanclub.org;
        client_max_body_size MAX_UPLOAD_SIZE;
        root /path/to/leanclub/;

        location / {
            proxy_pass http://127.0.0.1:18080;
            proxy_redirect off;
            proxy_set_header X-Forwarded-For $http_x_forwarded_for;
            proxy_set_header Host leanclub.org;
            add_header X-XSS-Protection "1; mode=block";
            add_header X-Frame-Options DENY;
        }
        	
        # Cache the avatar (Optional) 
        # location /info/ {
        #    proxy_pass http://127.0.0.1:18080;
        #    proxy_redirect off;
        #    proxy_set_header X-Forwarded-For $http_x_forwarded_for;
        #    proxy_set_header Host leanclub.org;
        #    proxy_cache one;
        #    proxy_cache_key "$request_uri";
        #    proxy_cache_valid  200 302  120m;
        # }

        location /static/ {
            expires 10d;
        }
        location /attachments/ {
            expires 365d;
        }
    }   
reverse proxy 127.0.0.1:18080 , and map the "static" and "attachments" folder.

# About
This program based on: [Crow](https://github.com/ipkn/crow)

I’m newbie for c++ , the code may “buggy" , not recommended for production.

### License
MIT


