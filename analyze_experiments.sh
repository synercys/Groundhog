#!/bin/bash
###############################################################################
# This script looks at the data logs from the docker container and extracts
# the line containing the encryption performance. The encryption performance 
# numbers are written onto a *_summary.txt file. 
#
# The performance numbers are then analyzed for mean and standard deviation 
# The mean and standard deviation are written onto *analysis file and then
# sorted and written onto a *stats file
###############################################################################
results_dir="data/results/blockchain"
#Make a summary directory and a stats directory
mkdir -p "${results_dir}/summary"
mkdir -p "${results_dir}/stats"

analysis_file="${results_dir}/stats/temp_perf_stats.txt"
groundhog_analysis_file="${results_dir}/stats/temp_perf_stats_groundhog.txt"
stats_file="${results_dir}/stats/perf_stats.txt"
groundhog_stats_file="${results_dir}/stats/perf_stats_groundhog.txt"

echo $analysis_file
echo $groundhog_analysis_file

enc_pos=7
echo "N", "M", "Mean", "Standard-Deviation" | tee $analysis_file
echo "N", "M", "Mean", "Standard-Deviation" | tee $groundhog_analysis_file

for FILE in "$results_dir"/*;do
	if [ ! -d "$FILE" ]; then
		echo "Analyzing file" $FILE
		N=`echo $FILE | grep -o "N[0-9]*" | sed 's/N//'`  # extract number of nodes
		M=`echo $FILE | grep -o "M[0-9]*" | sed 's/M//'`  # extract number of threshold nodes
		#echo $N, $M

		#grep -r '\bNet\b' $FILE | sed 's/\s\s*/ /g' | cut -d ' ' -f $enc_pos | cut -d ':' -f 2 | awk '{ total += $1; count++ } END { printf("%f\n", total/count) }'
	
		myNumbers=`grep -r '\bNet\b' $FILE | sed 's/\s\s*/ /g' | cut -d ' ' -f $enc_pos | cut -d ':' -f 2`
		
		#if grep -q groundhog "$FILE"
		if [[ "$FILE" == *"groundhog"* ]];
		then
			#echo pattern found
			echo "Pattern found"
			echo "${myNumbers}" > "${results_dir}/summary/groundhog_N${N}_M${M}_summary.txt"
		else
			#echo pattern not found
			echo "Pattern Not found"
			echo "${myNumbers}" > "${results_dir}/summary/N${N}_M${M}_summary.txt"
		fi
		
	
		#Mean
		mean=$(echo "$myNumbers" | awk '{ total += $1; count++ } END { printf("%f\n", total/count) }')
	
		#Standard Deviation
		standardDeviation=$(echo "$myNumbers" | awk '{sum+=$1; sumsq+=$1*$1}END{print sqrt(sumsq/NR - (sum/NR)**2)}')

		#if grep -q "groundhog" "$FILE"
		if [[ "$FILE" == *"groundhog"* ]];
		then
			echo $N, $M, $mean, $standardDeviation | tee -a $groundhog_analysis_file
			echo "----------------"
		else
			echo $N, $M, $mean, $standardDeviation | tee -a $analysis_file
			echo "----------------"

		fi
	fi
done
sort -n -k1,1 -k2,2  $analysis_file  > $stats_file
sort -n -k1,1 -k2,2  $groundhog_analysis_file  > $groundhog_stats_file
