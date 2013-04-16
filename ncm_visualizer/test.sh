cat ../samples/rx1.bytecode | ../ncm_translator/ncm_translator > ../samples/rx1.ncm
cat ../samples/rx1.ncm |./parse.py > data.json
xdg-open visualize.html
