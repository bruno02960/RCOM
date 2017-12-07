#!/bin/bash

ifconfig eth1 down
ifconfig eth1 up
ifconfig eth1 172.16.11.253/24 #172.16.y1.253/24
                                              
echo 1 > /proc/sys/net/ipv4/ip_forward
echo 0 > /proc/sys/net/ipv4/icmp_echo_ignore_broadcasts

#ifconfig #check MAC address for eth0 and eth1

#route -n
