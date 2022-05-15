#! /bin/sh
#Verify whole word in the train dir
#verify.sh <AUDIO_DIR> <WORD>
#verify.sh audio/train/scarlet/ wake

DIR="$1"
WORD=$2
VLC_OPT="-q --gain 5 --play-and-exit --no-repeat --no-loop"

WORD_NUM=$(grep -n "$WORD" word_list | awk -F: '{print $1}')
WORD_NUM=$(($WORD_NUM-1))
function convert2word()
{
	INDEX=$(($1+1))
	sed "${INDEX}q;d" word_list
}

find "$DIR" -type f -name "${WORD_NUM}_*" > list_file
find "$DIR" -type f -name "*_${WORD_NUM}_*" >> list_file
find "$DIR" -type f -name "*_${WORD_NUM}.*" >> list_file
COUNT=$(cat list_file | wc -l)
I_COUNT="1"
printf "Press any key to remove\n\n"

while read FILE_PATH; do
	
	FILENAME=$(echo "$FILE_PATH" | awk -F '/' '{print $NF}')
	printf "\r\e[K\r$I_COUNT/$COUNT <$FILENAME> "
	python3 scripts/train/print_name.py "$FILENAME"
	printf "\e[K\r"
	sleep 0.5
	cvlc $VLC_OPT $FILE_PATH 2>/dev/null & 
	
	#aplay $FILE_PATH

	for i in {1..40}; do
		read -rs -N1 -t0.1 REP </dev/tty
		
		if [[ "$REP" ]]; then
			printf "\e[A\e[K\r$FILEBASE Removed: $WORD1 $WORD2 $WORD3\n\n"
			break
		fi
		printf "|"
	done
	printf "\e[A"
	I_COUNT=$(( I_COUNT + 1 ))
	
done <list_file

printf "\n"
rm list_file
