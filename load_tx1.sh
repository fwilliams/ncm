make all install vm=1
echo "stop" > /sys/network_code/control
cat samples/tx1.ncm > /sys/network_code/code 
cat samples/tx1.ncmp > /sys/network_code/params
echo "start" > /sys/network_code/control
