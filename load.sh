make -C ncm_translator

translate='false'
parseparams='false'
start='false'

while getopts tps opt; do
  case $opt in
  t)
      translate='true'
      ;;
  p)
      parseparams='true'
      ;;
  s)
      start='true'
      ;;
  \?)
      echo "Invalid option: -$OPTARG" >&2
      ;;
  esac
done

shift $((OPTIND - 1))

if [ -z $1 -o -z $2 ]
	then
		echo "usage: ./load.sh -t (translate) -p (compile params) -s (start) rx1.bytecode rx1.params.json"
	else
		if [ $translate == 'true' ]
			then
				cat $1 | ncm_translator/ncm_translator > /sys/network_code/code
			else
				cat $1 > /sys/network_code/code 
		fi
		if [ $parseparams == 'true' ]
			then
				cat $2 | python ncm_translator/mkparams.py > /sys/network_code/params
			else
				cat $2 > /sys/network_code/params
		fi
		if [ $start == 'true' ]
			then
				echo "run" > /sys/network_code/control
			if [ `cat /sys/network_code/control` == 'running' ]
				then
					echo "Network code machine is now running."
				else
					echo "Failed to start network code machine."
			fi
		fi
fi
