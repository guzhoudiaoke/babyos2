#!/bin/sh
brctl addbr br0
brctl addif br0 enp3s0
brctl stp br0 off

ifconfig br0 192.168.1.2 promisc up
ifconfig enp3s0 192.168.1.103 promisc up
