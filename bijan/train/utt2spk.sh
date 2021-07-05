#!/bin/bash
# Generate utt2spk from audio directory
# Usage: utt2spk <audio_dir> <output_dir>

AUDIO_DIR="$1"
OUTPUT="$2/utt2spk"
	
if [ -f list_file ]; then
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
		SPK_ID=$(echo "$f" | awk -F '/' '{print $(NF-1)}')
		printf "$WAV_NAME $SPK_ID\n" >> "$OUTPUT"
	done <list_file
	
	rm list_file
		
done <list_dir

rm list_dir
