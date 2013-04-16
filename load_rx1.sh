make all install
make -C ncm_translator
#cat samples/rx1.params.json | python ncm_translator/mkparams.py > samples/rx1.ncmp
#echo "stop" > /sys/network_code/control
cat samples/rx1.bytecode | ncm_translator/ncm_translator > /sys/network_code/code 
cat samples/rx1.ncmp > /sys/network_code/params
echo "run" > /sys/network_code/control
