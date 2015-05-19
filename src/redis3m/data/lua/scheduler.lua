local res = redis.call('ZRANGEBYSCORE',KEYS[1],0,ARGV[1],'LIMIT',0,1)

if #res > 0 then
  redis.call('ZADD', KEYS[1], ARGV[2], res[1])
  return res[1]
else
  return false
end
