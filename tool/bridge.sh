ifconfig enp3s0 down
brctl addbr br0
brctl addif br0 enp3s0
brctl stp br0 off
ifconfig br0 0.0.0.0 promisc up
ifconfig enp3s0 0.0.0.0 promisc up
dhclient br0
tunctl -t tap0 -u liuruyi
tunctl -t tap1 -u liuruyi
brctl addif br0 tap0
brctl addif br0 tap1
ifconfig tap0 0.0.0.0 promisc up
ifconfig tap1 0.0.0.0 promisc up
