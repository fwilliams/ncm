make all install
cat samples/tx1.bytecode | ncm_translator/ncm_translator > samples/tx1.ncm
cat samples/tx1.params.json | python ncm_translator/mkparams.py > samples/tx1.ncmp
#echo "stop" > /sys/network_code/control
cat samples/tx1.ncm > /sys/network_code/code 
cat samples/tx1.ncmp > /sys/network_code/params
echo "run" > /sys/network_code/control
