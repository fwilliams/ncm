make all install vm=2
cat /sys/network_code/code > samples/vm2.ncm
cat /sys/network_code/params > samples/vm2.ncmp
make uninstall vm=2
