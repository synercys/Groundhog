#!/bin/bash
data_dir="/data/results/profiling"

mean_pos=5
stddev_pos=7
for FILE in "${data_dir}/*";do
	if [! -d "$FILE"];then
		echo "Analyzing file" $FILE
		N=`echo $FILE | grep -o "N[0-9]*" | sed 's/N//'`
		M=`echo $FILE | grep -o "M[0-9]*" | sed 's/M//'`

		oprf-send-times-means=`grep -e 'OPRF-Send-Times' data/results/profiling/N4_M2.txt | sed 's/\s\s*/ /g' | cut -d ' ' -f $mean_pos | cut -d ':' -f 2`
		
		oprf-send-times-stddevs=`grep -e 'OPRF-Send-Times' data/results/profiling/N4_M2.txt | sed 's/\s\s*/ /g' | cut -d ' ' -f $stddev_pos | cut -d ':' -f 2`

		#Mean of Means
		oprf-send-times-mean=$(echo "oprf-send-times-means" | awk '{ total += $1; count++ } END {printf("%f\n", total/count)}')
	


