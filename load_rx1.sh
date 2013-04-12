make all install vm=1
echo "stop" > /sys/network_code/control
cat samples/rx1.ncm > /sys/network_code/code 
cat samples/rx1.ncmp > /sys/network_code/params
echo "run" > /sys/network_code/control
