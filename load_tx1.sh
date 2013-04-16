make all install
make -C ncm_translator
cat samples/tx1.bytecode | ncm_translator/ncm_translator > /sys/network_code/code 
cat samples/tx1.params.json | python ncm_translator/mkparams.py > /sys/network_code/params
echo "run" > /sys/network_code/control
