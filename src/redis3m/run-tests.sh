#!/bin/bash

# Standalone instance
redis-server --port 6379 --save "" &> /dev/null &

# HA instaces with sentinel
for x in 6380 6381 6382; do
  redis-server --port $x --save "" &> /dev/null &
done
sleep 0.5
for x in 6381 6382; do
  redis-cli -p $x slaveof 127.0.0.1 6380
done
sleep 0.5
for x in 6380 6381 6382; do
  redis-cli -p $x role
done
for x in 26379 26380 26381; do
  redis-server tests/redis-sentinel-$x.conf --sentinel --port $x &> /dev/null &
done
sleep 1

redis-cli -p 26379 info sentinel
redis-cli -p 26379 sentinel get-master-addr-by-name test

# Run tests
REDIS_HOST=localhost ctest -V
RESULT=$?

# Cleanup all instances
jobs -p | xargs kill

exit $RESULT
