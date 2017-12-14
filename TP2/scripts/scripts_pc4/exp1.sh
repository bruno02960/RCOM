#!/bin/bash

/etc/init.d/networking restart
ifconfig eth0 down #reset config of eth0
ifconfig eth0 up
ifconfig eth0 172.16.30.254/24

#For config check
#ifconfig 
#route -n

#For table checking
#arp -a

#For table ip removal
#arp -d 172.16.30.254 

