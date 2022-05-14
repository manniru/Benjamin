#! /bin/sh
# Author: M. Abdollah Zade 2022
# ./verify_o.sh <MAX_DAY>

source scripts/functions/bt_kaldi.sh

AUD_DIR="audio/unverified"
VER_DIR="audio/train/online" #verified online
MAX_DIFF=$1
mkdir -p "$VER_DIR"

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

	FILE_DATE=$(stat $FILE | grep Change | cut -d ' ' -f2)
	FILE_DATA_NUM=$(date -d $FILE_DATE +%s)

	DATE_NOW=$(date +%s)
	DIFF=$(( $DATE_NOW - $FILE_DATA_NUM ))

	DIFF_DATE=$(( $DIFF / 86400 ))
	#exit 0

	if [[ $DIFF_DATE -lt $MAX_DIFF ]]; then
		checkAudio $FILE
	fi

done <list_file

rm list_file