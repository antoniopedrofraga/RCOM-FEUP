#!/bin/bash
 
ifconfig eth0 up 172.16.11.1/24
route add -net 172.16.10.0/24 gw 172.16.11.253
route add default gw 172.16.11.253
echo "tux2 configured"
