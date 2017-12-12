copy flash:tux3-clean startup-config 
#copy flash:gnu3-clean startup-config 
#copy flash:gnu(tux)Y-clean startup-config
reload
#fastethernet for room I320

#Router ports config with NAT
enable
conf term
interface gigabitethernet 0/0 #GE 0/0 config
#interface fastethernet 0/0 #GE 0/0 config
ip address 172.16.31.254 255.255.255.0
#ip address 172.16.y1.254 255.255.255.0
no shutdown
ip nat inside
exit

interface gigabitethernet 0/1 #GE 0/1 config
#interface fastethernet 0/1 #GE 0/1 config
ip address 172.16.2.39 255.255.255.0
#ip address 172.16.x.y9 255.255.255.0
#x -> sala(sala das tps=1, outra sala=2), y->bancada
no shutdown
ip nat outside
exit

#configure Router routes
#vlan y1 route
ip route 172.16.30.0 255.255.255.0 172.16.31.253 
#ip route 172.16.y0.0 255.255.255.0 172.16.y1.253 
ip route 0.0.0.0 0.0.0.0 172.16.2.254  #default route
#ip route 0.0.0.0 0.0.0.0 172.16.x.254
exit

#NAT CONFIG

ip nat pool ovrld 172.16.2.39 172.16.2.39 prefix 24
#ip nat pool ovrld 172.16.x.y9 172.16.x.y9 prefix 24
ip nat inside source list 1 pool ovrld overload

access-list 1 permit 172.16.30.0 0.0.0.7
#access-list 1 permit 172.16.y0.0 0.0.0.7
access-list 1 permit 172.16.31.0 0.0.0.7
#access-list 1 permit 172.16.y1.0 0.0.0.7
ip route 0.0.0.0 0.0.0.0 172.16.2.254
#ip route 0.0.0.0 0.0.0.0 172.16.x.254
ip route 172.16.30.0 255.255.255.0 172.16.31.253
#ip route 172.16.y0.0 255.255.255.0 172.16.y1.253
end


