#! /bin/sh
# Author: M. Abdollah Zade 2022
# ./verify_o.sh <MAX_DAY>

source scripts/functions/bt_kaldi.sh

AUD_DIR="audio/unverified"
MAX_DIFF=$1
#verified online
mkdir -p "audio/train/online"

# Find with date output
find "$AUD_DIR" -type f -printf "%T@ %Tc %p\n" > list_file
# sort by date
sort -n -o list_file list_file

while read LINE; do

	FILE=$(echo $LINE | cut -d ' ' -f9)
	printf "\r\e[K\r"
	FILENAME=$(echo "$FILE" | awk -F '/' '{print $NF}')
	python3 scripts/train/print_name.py "$FILENAME"
	printf "\e[A"

	FILE_DATE=$(stat $FILE | grep Change | cut -d ' ' -f2)
	# exit 0
	FILE_DATA_NUM=$(date -d $FILE_DATE +%s)

	DATE_NOW=$(date +%s)
	DIFF=$(( $DATE_NOW - $FILE_DATA_NUM ))

	DIFF_DATE=$(( $DIFF / 86400 ))

	if [[ $DIFF_DATE -lt $MAX_DIFF ]]; then
		checkAudio $FILE
	fi

done <list_file

rm list_file