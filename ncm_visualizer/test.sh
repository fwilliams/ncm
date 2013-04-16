cat ../samples/rx1.bytecode | ../ncm_translator/ncm_translator > ../samples/rx1.ncm
cat ../samples/tx1.bytecode | ../ncm_translator/ncm_translator > ../samples/tx1.ncm
echo 'receive_data({"rx":' > data.json
cat ../samples/rx1.ncm |./parse.py >> data.json
echo ', "tx":' >> data.json
cat ../samples/tx1.ncm |./parse.py >> data.json
echo '})' >> data.json
# xdg-open visualize.html
