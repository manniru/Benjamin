#!/bin/bash
# Generate wav.scp and utt2spk from audio directory
# Usage: wav_scp <audio_dir> <output_dir>

AUDIO_DIR="$1"
WAV_SCP="$2/wav.scp"
UTT2SPK="$2/utt2spk"

find "$AUDIO_DIR" -type d > list_dir
sed -i '1d' list_dir #remove first line

while read p; do
	
	find "$p" -type f > list_file
	
	while read f; do
	
		FILENAME=$(echo "$f" | awk -F '/' '{print $NF}')
		FILEBASE=$(echo "$FILENAME" | awk -F '.' '{print $1}')
		SPEAK_ID=$(echo "$f" | awk -F '/' '{print $(NF-1)}')

		WAV_NAME="${SPEAK_ID}_${FILEBASE}"
		echo "$WAV_NAME $f" >> "$WAV_SCP"
		echo "$WAV_NAME $SPEAK_ID" >> "$UTT2SPK"
		
	done <list_file
	
	rm list_file
		
done <list_dir

rm list_dir
