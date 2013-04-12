make all install vm=1
cat /sys/network_code/code > samples/vm1.ncm
cat /sys/network_code/params > samples/vm1.ncmp
make uninstall vm=1
