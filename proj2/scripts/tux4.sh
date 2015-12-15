#!/bin/bash

ifconfig up eth0 172.16.10.254/24
ifconfig up eth1 172.16.11.253/24
echo "tux4 configured"
