#!/bin/bash
# Generate wav.scp from audio directory
# Usage: wav_scp <audio_dir> <output_dir>

AUDIO_DIR="$1"
OUTPUT="$2/wav.scp"

if [ -f list_dir ]; then
	rm list_dir
fi

find "$AUDIO_DIR" -type f > list_file

while read f; do

	WAV_NAME=$(echo "$f" | awk -F '/' '{print "bijan_"$NF}' | awk -F '.' '{print $1}' )
	printf "$WAV_NAME $f\n" >> "$OUTPUT"

done <list_file

rm list_file

