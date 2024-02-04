#!/bin/bash

N=$1
for ((i=2; i<=$((N-2)); i++)); 
do
	echo "N="$N, "M="$i, "Num data pts="`grep -r '\bNet\b' data/results/profiling/N"${N}"_M${i}.txt | wc -l`;
done
