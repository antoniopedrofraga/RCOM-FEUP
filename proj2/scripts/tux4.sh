#!/bin/bash

ifconfig up eth0 172.16.10.254/24
ifconfig up eth1 172.16.11.253/24
route add default gw 172.16.11.254
printf "search lixa.fe.up.pt\nnameserver 172.16.1.1\n" > /etc/resolv.conf
echo "tux4 configured"
