#! /bin/sh
#Verify whole word in the train dir
#verify.sh <AUDIO_DIR> <WORD>
#verify.sh audio/train/scarlet/ wake

DIR="$1"
WORD=$2
VLC_OPT="-q --gain 5 --play-and-exit --no-repeat --no-loop"

FILE_NUM=$(grep -n "$WORD" word_list | awk -F: '{print $1}')
FILE_NUM=$(($FILE_NUM-1))
function convert2word()
{
	INDEX=$(($1+1))
	sed "${INDEX}q;d" word_list
}

find "$DIR" -type f -name "$FILE_NUM*" > list_file

printf "Press any key to remove\n\n"

while read FILE_PATH; do
	
	FILENAME=$(echo "$FILE_PATH" | awk -F '/' '{print $NF}')
	FILEBASE=$(echo "$FILENAME"  | awk -F '.' '{print $1}')
	INDEX1=$(echo "$FILEBASE"    | awk -F '_' '{print $1}')
	INDEX2=$(echo "$FILEBASE"    | awk -F '_' '{print $2}')
	INDEX3=$(echo "$FILEBASE"    | awk -F '_' '{print $3}')

	WORD1=$(convert2word $INDEX1)
	WORD2=$(convert2word $INDEX2)
	WORD3=$(convert2word $INDEX3)
	
	# \e[A: one line up - \e[K: clear line
	printf "\e[A\e[K\rProcessing = $WORD1 $WORD2 $WORD3     \n"
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
	
done <list_file

printf "\n"
rm list_file
