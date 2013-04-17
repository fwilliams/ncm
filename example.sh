# This unloads the module if it's already loaded
if [ -a /sys/network_code ]
	then
		make uninstall
fi

# Build the network code module
make all
# Load the module
make install

# Read bytecomes in the hardware format and traslate (-t) them
# Then read a .params.json file and convert it (-p) into a binary params file
# Then start (-s) the network code machine
./load.sh -tps samples/rx1.bytecode samples/rx1.params.json

# Check what's going on
dmesg | tail
echo -n "Network code machine: "
cat /sys/network_code/control

# Write the date into a variable
# date > /sys/network_code/varspace/1

# Read the date from a variable
# cat /sys/network_code/varspace/1