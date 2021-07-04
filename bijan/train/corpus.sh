#!/bin/bash

AUDIO_DIR="$1"
FILE_TRAIN="$2"

STRING_DIGIT=(zero one two three four five six seven eight nine)

find "$AUDIO_DIR" -type d > list_dir
sed -i '1d' list_dir #remove first line

while read p; do
	
	if [ -f list_file ]; then
		rm list_file
	fi
	
	find "$p" -type f > list_file
	
	while read f; do
		WAV_NAME=$(echo "$f" | awk -F '/' '{print $(NF-1)"_"$NF}' | awk -F '.' '{print $1}' )
		DIGIT1=$(echo "$f" | awk -F '/' '{print $NF}' | awk -F '.' '{print $1}' | awk -F '_' '{print $1}' )
		DIGIT2=$(echo "$f" | awk -F '/' '{print $NF}' | awk -F '.' '{print $1}' | awk -F '_' '{print $2}' )
		DIGIT3=$(echo "$f" | awk -F '/' '{print $NF}' | awk -F '.' '{print $1}' | awk -F '_' '{print $3}' )
		printf "${STRING_DIGIT[$DIGIT1]} ${STRING_DIGIT[$DIGIT2]} ${STRING_DIGIT[$DIGIT3]}\n" >> "$FILE_TRAIN"
	done <list_file
	
	rm list_file
		
done <list_dir
