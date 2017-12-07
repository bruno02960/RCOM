#!/bin/bash

/etc/init.d/networking restart
ifconfig eth0 down #reset config of eth0
ifconfig eth0 up
ifconfig eth0 172.16.11.1/24 #172.16.y1.1/24

#ping -b 172.16.31.255

