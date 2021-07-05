#!/bin/bash
# Generate wav.scp from audio directory
# Usage: wav_scp <audio_dir> <output_dir>

AUDIO_DIR="$1"
OUTPUT="$2/wav.scp"

if [ -f list_dir ]; then
	rm list_dir
fi

find "$AUDIO_DIR" -type d > list_dir
sed -i '1d' list_dir #remove first line

while read p; do
	
	if [ -f list_file ]; then
		rm list_file
	fi
	
	find "$p" -type f > list_file
	
	while read f; do
		WAV_NAME=$(echo "$f" | awk -F '/' '{print $(NF-1)"_"$NF}' | awk -F '.' '{print $1}' )
		printf "$WAV_NAME $f\n" >> "$OUTPUT"
	done <list_file
	
	rm list_file
		
done <list_dir

rm list_dir
