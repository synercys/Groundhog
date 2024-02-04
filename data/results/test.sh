#!/bin/bash
for file in groundhog_N16* ; do
	#echo "$(sed -i 's/groundhog_//g' "$file")"
	mv "$file" "${file/groundhog_/}"
	#mv $file $(echo $file | rev | cut -f2- -d- | rev).pkg
    done
