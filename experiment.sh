#!/bin/bash
##############################################################################
# This script automates the run_tests.sh script by looping through different
# $N$ and $M$. The argument 'default' or 'groundhog' depends on the scheme
#
# The timeout command is used to forcefully stop the tests after a 120 seconds
# Going forward, we hope the htdise_uptime_1 docker containers exit cleanly 
# There are 10 iterations of run_tests.sh
###############################################################################
sync;
echo 3 > /proc/sys/vm/drop_caches;
arg=""
data_dir="data/results/blockchain"
mkdir -p "${data_dir}"

# Check if the number of arguments is zero
if [ $# -eq 0 ]
  then
    echo "No arguments supplied - One argument needed - that should be either 'groundhog' or 'default'"
  else
	# Convert arguments into lowercase to accept Groundhog, groundhog
    arg="$(echo "$1" | tr '[A-Z]' '[a-z]')"
fi
# Echo the argument to check if it has been converted into lower-case
echo "$arg"
# Bash v4.0 onwards has support for step value using {START..END..INCREMENT} syntax
# Example: for n in {4..20..4}
for n in 20;do
	# Forcing ctr=2 for reviewer's protocol
	#for((ctr=2; ctr<=3; ctr++));
	for((ctr=8; ctr<=$((n-2)); ctr=ctr+2));
	do
		if [[ "${arg}" == "default" ]]
		then
			echo "Preparing to run default scheme"
			# Clearing any residual data
			echo > "${data_dir}"/N"${n}"_M"${ctr}".txt;
		elif [[ "${arg}" == "groundhog" ]]
		then
			echo "Preparing to run groundhog scheme"
			# Clearing any residual data
			echo > "${data_dir}"/groundhog_N"${n}"_M"${ctr}".txt;
		else
			# Invalid arguments
			echo "Check for typo in the arguments"
		fi

		for iter in {1..3};
		do
			# N=24: 6 mins for t=7, 15 mins for t=8
			if [[ "${arg}" == "default" ]]
			then
				echo "Running expt. for default"
				timeout 300 ./run-tests.sh $n $ctr --build &>> "${data_dir}"/N"${n}"_M"${ctr}".txt;
			elif [[ "${arg}" == "groundhog" ]]
			then
				echo "Running expt. for groundhog"
				timeout 60 ./run-tests.sh $n $ctr --build &>> "${data_dir}"/groundhog_N"${n}"_M"${ctr}".txt;
			fi
			sync;
			echo 3 > /proc/sys/vm/drop_caches;
		done;
	done;
done;
