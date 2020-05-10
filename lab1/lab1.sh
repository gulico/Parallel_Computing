#!/bin/bash

seq="./lab1-seq"
para="./para/lab1-par-4"

starttime=`date +'%Y-%m-%d %H:%M:%S'`
find . -type f -name '*.txt' | xargs rm

if [ ! -f "$seq" ]; then
	gcc-9 -O3 -Wall -Werror -o lab1-seq lab1.c -lm
fi



if [ ! -f "$para" ]; then
	for loop in 2 3 4 5
	do
		gcc-9 -O3 -Wall -Werror -floop-parallelize-all -ftree-parallelize-loops=$loop lab1.c -o ./para/lab1-par-$loop -lm
	done
fi


N1=0
N2=0
delta1=10
delta2=2000
max=10000
loop=0
while [ ${loop} -le 50000 ]
do
	let loop+=2500

	time=$(./lab1-seq $loop) 

	echo "N:$loop --X:"${time}"ms" >> res_log.txt

	realtim=${time#*:}
	echo "loop: "${loop}"--using time"${realtim} >> ./seq_log.txt

	echo "N:"${loop}"--using time"$realtim"ms"

	if [ ${realtim} -gt $delta2 ]; then # > 2s
		if [ ${N2} == 1 ];then
			break
		else
			N2=1
			echo "N2 is ${loop}, and using time aroud 2000ms"
			echo "N2->loop: "${loop}--"using time"${realtim} >> N_log.txt
		fi		
	fi

	if [ ${realtim} -gt $delta1 ]; then # > 0.01s
		if [ ${N1} == 1 ];then
			continue
		else
			N1=1
			echo "N1 is ${loop}, and using time aroud 10ms"
			echo "N1->loop: "${loop}--"using time"${realtim} >> N_log.txt
		fi		
	fi
done

for i in 2 3 4 5
do
	loop=0	
	while [ ${loop} -le 27500 ]
	do
		let loop+=2500

		time=$(./para/lab1-par-${i} $loop) 

		echo "N:$loop --X:"${time}"ms" >> res_log.txt

		realtim=${time#*:}
		echo "N: "${loop}"--using time"${realtim}"ms" >> ./para/${i}para_log.txt
	done
done

endtime=`date +'%Y-%m-%d %H:%M:%S'`
start_seconds=$(date --date="$starttime" +%s);
end_seconds=$(date --date="$endtime" +%s);
echo "本次运行时间： "$((end_seconds-start_seconds))"s"
