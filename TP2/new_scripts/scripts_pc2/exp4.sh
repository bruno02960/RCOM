#!/bin/bash

route add -net 0.0.0.0 gw 172.16.11.254
#route add -net 0.0.0.0 gw 172.16.y1.254

echo > 1 /proc/sys/net/ipv4/conf/all/accept redirects
echo > 1 /proc/sys/net/ipv4/conf/eth0/accept redirects
