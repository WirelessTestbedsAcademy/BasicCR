#!/bin/bash

#Run this script with the interface as the first parameter. For example with interface eth0, run "./setupusrp2 eth0"

#Bring up the interface and set IP
sudo ifconfig $1 up
sudo ifconfig $1 192.168.10.1 netmask 255.255.255.0 
echo Interface up

#Add a route to direct all usrp traffic to the correct interface
sudo route add -net 192.168.10.0 netmask 255.255.255.0 gw 192.168.10.2 dev $1
echo Route added

#Turn on flow control on the interface
sudo ethtool -A $1 rx on
echo Flow control on


