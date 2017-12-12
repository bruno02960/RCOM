#clean configuration
enable
conf term
no vlan 10 # no vlan y0
no vlan 11 # no vlan y1
end
copy flash:tux1-clean startup-config
#copy flash:gnuY-clean startup-config
#copy flash:tuxY-clean startup-config
reload

#implement vlans
enable
conf term
vlan 10 # vlan y0
exit
vlan 11 # vlan y1
exit

#interface fastethernet 0/P

#For vlan y0
interface fastethernet 0/1
#0/x -> port do switch range 0/1 a 0/24
switchport mode access
switchport access vlan 10 #y0
exit
interface fastethernet 0/2 #0/x
switchport mode access
switchport access vlan 10 #y0
exit

#For vlan y1
interface fastethernet 0/13 #0/x
switchport mode access
switchport access vlan 11  #y1
exit
interface fastethernet 0/14 #0/x
switchport mode access
switchport access vlan 11 #y1
exit
interface fastethernet 0/15 #0/x
switchport mode access
switchport access vlan 11 #y1
exit

end

#For checking configs
show running-config
show vlan
show interface status
