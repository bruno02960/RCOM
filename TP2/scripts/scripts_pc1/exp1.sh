#!/bin/bash

/etc/init.d/networking restart
ifconfig eth0 down #reset config of eth0
ifconfig eth0 up
ifconfig eth0 172.16.30.1/24 #172.16.y0.1/24

#For config check
#ifconfig 
#route -n

#For testing tux4
#ping 172.16.10.254 #172.16.y0.254  

#arp -a

#arp -d 172.16.30.254 

