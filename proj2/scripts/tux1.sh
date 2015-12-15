#!/bin/bash

ifconfig eth0 up 172.16.10.1/24
route add -net 172.16.11.0/24 gw 172.16.10.254
route add default gw 172.16.10.254
printf "search lixa.fe.up.pt\nnameserver 172.16.1.1\n" > /etc/resolv.conf
echo "tux1 configured"
