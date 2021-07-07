#!/bin/bash
# Generate utt2spk from audio directory
# Usage: utt2spk <audio_dir> <output_dir>

AUDIO_DIR="$1"
OUTPUT="$2/utt2spk"

find "$AUDIO_DIR" -type f > list_file

while read f; do

	WAV_NAME=$(echo "$f" | awk -F '/' '{print "bijan_"$NF}' | awk -F '.' '{print $1}' )
	SPK_ID="bijan"
	printf "$WAV_NAME $SPK_ID\n" >> "$OUTPUT"

done <list_file

rm list_file
