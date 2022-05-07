#! /bin/sh
# Author: M. Abdollah Zade 2022
# ./verify_o.sh <MAX_DAY>

source scripts/functions/bt_kaldi.sh

AUD_DIR="audio/train/online"
MAX_DIFF=$1
	
find "$AUD_DIR" -type f > list_file

while read FILE; do

	FILE_DATE=$(stat $FILE  | grep Change | cut -d ' ' -f2)
	FILE_DATA_NUM=$(date -d $FILE_DATE +%s)

	DATE_NOW=$(date +%s)
	DIFF=$(( $DATE_NOW - $FILE_DATA_NUM ))

	DIFF_DATE=$(( $DIFF / 86400 ))

	if [[ $DIFF_DATE -lt $MAX_DIFF ]]; then
		checkAudio $FILE
	fi

done <list_file

rm list_file