for vm in `ls | grep '\.bytecode' | sed 's/\.bytecode//'`
do
	cat ${vm}.bytecode | ../ncm_translator/ncm_translator > ${vm}.ncm
done
