cat ../samples/tx1.bytecode | ../ncm_translator/ncm_translator > ../samples/tx1.ncm
cat ../samples/tx1.ncm |./parse.py > data.json
