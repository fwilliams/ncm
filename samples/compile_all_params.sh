for vm in `ls | grep '\.params\.json' | sed 's/\.params\.json//'`
do
	cat ${vm}.params.json | ../ncm_translator/mkparams.py > ${vm}.ncmp
done
