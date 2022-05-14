#! /bin/sh
# Author: M. Abdollah Zade 2022
# ./verify_o.sh

source scripts/functions/bt_kaldi.sh

AUD_DIR="audio/unverified"
VER_DIR="audio/train/online" #verified online
mkdir -p "$VER_DIR"
clear

# Find with date output
find "$AUD_DIR" -type f -printf "%T@ %Tc %p\n" > list_file
# sort by date
sort -n -o list_file list_file
printf "\n"
while read LINE; do

	FILE=$(echo $LINE | cut -d ' ' -f9)
	U_COUNT=$(find "$AUD_DIR" -type f | wc -l)
	V_COUNT=$(find "$VER_DIR" -type f | wc -l)
	printf "\r\e[K\r<$U_COUNT->$V_COUNT> "
	FILENAME=$(echo "$FILE" | awk -F '/' '{print $NF}')
	python3 scripts/train/print_name.py "$FILENAME"
	
	printf "\e[A"
	checkAudio $FILE

done <list_file

rm list_file