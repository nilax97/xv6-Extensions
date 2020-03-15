!/bin/bash
pkill qemu-system-x86
make clean
make fs.img
make

# make qemu 
echo "Running..1"
./test_assig1.sh assig1_1|grep -i 'sys_'|sed 's/$ //g'|sort > res_assig1_1

echo "Running..2"
./test_assig1.sh assig1_2|grep -i 'sys_'|sed 's/$ //g'|sort > res_assig1_2

echo "Running..3"
./test_assig1.sh assig1_3|grep -i Sum|sed 's/$ //g'|sort > res_assig1_3

echo "Running..4"
./test_assig1.sh assig1_4|grep -i Sum|sed 's/$ //g'|sort > res_assig1_4

echo "Running..5"
./test_assig1.sh assig1_5|grep -i pid|sed 's/$ //g'|sort > res_assig1_5

echo "Running..6"
./test_assig1.sh assig1_6|grep -i pid|sed 's/$ //g'|sort > res_assig1_6

echo "Running..7"
./test_assig1.sh assig1_7|grep -i 'PARENT\|CHILD'|sed 's/$ //g'|sort> res_assig1_7

echo "Running..8 (this will take 10 seconds)"
./test_assig1_long.sh assig1_8 0 arr|grep -i 'Sum of array'|sed 's/$ //g'|sort> res_assig1_8

echo "Running..9 (this will take 10 seconds)"
./test_assig1_long.sh assig1_8 1 arr|grep -i 'Variance of array'|sed 's/$ //g'|sort> res_assig1_9


check_test=9
total_test=0

for ((t=1;t<=$check_test;++t))
do
	echo -n "Test #${t}: "

	# NOTE: we are doing case insensitive matching.  If this is not what you want,
	# just remove the "-i" flag
	if diff -iZ <(sort out_assig1_$t) <(sort res_assig1_$t) > /dev/null
	then
		echo -e "\e[0;32mPASS\e[0m"
		((total_test++))
	else
		echo -e "\e[0;31mFAIL\e[0m"
	fi
done
echo "$total_test" test cases passed
