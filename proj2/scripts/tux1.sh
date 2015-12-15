#!/bin/bash

ifconfig eth0 up 172.16.10.1/24
route add -net 172.16.11.0/24 gw 172.16.10.254
route add default gw 172.16.10.254
echo "tux1 configured"
