#!/bin/bash

AUDIO_DIR="$1"
SPK2G_FILE="$2"

find "$AUDIO_DIR" -type d | awk -F '/' '{print $(NF)'} > list_dir
sed -i '1d' list_dir #remove first line

while read p; do
	printf "$p m\n" >> "$SPK2G_FILE"		
done <list_dir

rm list_dir
