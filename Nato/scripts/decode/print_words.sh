#!/bin/bash

function comp_f
{
	if (( $(echo "$1 > $2" |bc -l) )); then
		echo "1"
	fi
}

DECODE_PATH=$1
THRESHOLD=$2

MODE="tri1"
WORD_TABLE="exp/$MODE/graph/words.txt"

RESULT_PATH="$DECODE_PATH/result"
CONF_FILE="$RESULT_PATH/confidence"

SUM=$(wc -l $CONF_FILE | awk '{ print $1 }')
LOST_SUM="0"
CONF_SUM="0"

if [[ "$SUM" -eq 0 ]]; then
	exit 0
fi

while read line; do

	CONF=$(echo $line | awk '{print $6}')
	IS_GT=$(comp_f $CONF $THRESHOLD)
	
	if [[ -n "$IS_GT" ]]; then
	
		LOST_SUM=$(($LOST_SUM + 1))
		CONF_SUM=$(bc <<< "scale=2; $CONF+$CONF_SUM")
	
	fi

done <$CONF_FILE

UTT_CONF=$(bc <<< "scale=2; $LOST_SUM/$SUM")
IS_GT=$(comp_f $UTT_CONF "0.5")

if [[ -z "$IS_GT" ]]; then
	
	UTT_MEAN=$(bc <<< "scale=2; $CONF_SUM/$SUM")
	echo "LOW UTT_CONF $UTT_CONF $UTT_MEAN"
	#exit 0
	
fi

while read line; do

	INDEX=$(echo $line | awk '{print $5}')
	CONF=$(echo $line | awk '{print $6}')
	WORD=$(grep $INDEX $WORD_TABLE | awk '{print $1}')
	IS_GT=$(comp_f $CONF $THRESHOLD)
	
	if [[ "$IS_GT" ]]; then
	
		printf "$WORD "
	
	else
	
		printf "($CONF) "
	
	fi

done <$CONF_FILE

printf "\n"

# echo "UTT_CONF=$UTT_CONF"

if [[ "$#" -gt "3" ]]; then

	while read line; do

		INDEX=$(echo $line | awk '{print $5}')
		CONF=$(echo $line | awk '{print $6}')
		WORD=$(grep $INDEX $WORD_TABLE | awk '{print $1}')
		IS_GT=$(comp_f $CONF $THRESHOLD)
		
		if [[ "$IS_GT" ]]; then
		
			printf "%6s" $WORD 
		
		else
		
			printf "%6s" "($CONF)"
		
		fi

	done <$CONF_FILE

	while read line; do

		CONF=$(echo $line | awk '{print $6}')
		printf "%6s " $CONF

	done <$CONF_FILE

	printf "\n"

fi
