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






ifconfig enp3s0 down
sudo ifconfig enp3s0 down
ping baidu.com
sudo ifconfig br0 down
sudo tunctl -d tap0
sudo tunctl -d tap1
brctl delbr br0
sduo brctl delbr br0
sudo brctl delbr br0
brctl addbr br0
sudo brctl addbr br0
brctl addif br0 enp3s0
sudo brctl addif br0 enp3s0
brctl stp br0 off
sudo brctl stp br0 off
ifconfig br0 0.0.0.0 promisc up
sudo ifconfig br0 0.0.0.0 promisc up
sudo ifconfig enp3s0 0.0.0.0 promisc up
dhclient br0
sudo dhclient br0
brctl show br0
brctl showstp br0
route
ping baidu.com
ping ping 192.168.1.1
ping -I enp3s0 baidu.com
ping -I br0 baidu.com
route del default gw 192.168.1.1 dev enp3s0
sudo route del default gw 192.168.1.1 dev enp3s0
ping baidu.com
ping 192.168.1.1
