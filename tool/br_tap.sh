#!/bin/sh
sudo brctl addbr br0
sudo brctl addif br0 enp3s0
sudo brctl stp br0 off
sudo 
sudo ifconfig br0 192.168.1.2 promisc up
sudo ifconfig enp3s0 192.168.1.103 promisc up

#tunctl -d tap0
#sudo tunctl -t tap0 -u liuruyi
#sudo ifconfig tap0 0.0.0.0 promisc up






sudo brctl addbr br0
sudo brctl addif br0 enp3s0
sudo brctl stp br0 off
sudo ifconfig br0 192.168.1.101 promisc up
sudo ifconfig br0 hw ether ac:22:0b:29:df:8d

sudo tunctl -t tap0 -u liuruyi
sudo tunctl -t tap1 -u liuruyi
sudo brctl addif br0 tap0
sudo brctl addif br0 tap1
sudo ifconfig tap0 0.0.0.0 promisc up
sudo ifconfig tap1 0.0.0.0 promisc up
