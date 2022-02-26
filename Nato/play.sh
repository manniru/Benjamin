#!/bin/bash

CAT_NAME="scarlet"
AUDIO_PATH="audio/train/$CAT_NAME"
VLC_OPT="-q --gain 5 --play-and-exit --no-repeat --no-loop"

function convert2word()
{
	INDEX=$(($1+1))
	sed "${INDEX}q;d" word_list
}

function play_data()
{
	FILEBASE=$1
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
	cvlc $VLC_OPT "$AUDIO_PATH/$FILEBASE.wav" 2>/dev/null & 

	for i in {1..40}; do
		read -rs -N1 -t0.1 REP </dev/tty
		
		if [[ "$REP" ]]; then
		
			printf "\e[K\rPress Any Key To Remove $FILEBASE, press q to abort"
			read -rs -N1 REP </dev/tty
			if [[ "$REP" != "q" ]]; then
				printf "\e[A\e[K\r$FILEBASE Removed: $WORD1 $WORD2 $WORD3\n\n"
				rm "$AUDIO_PATH/$FILEBASE.wav"
			fi
			break
		fi
		
		printf "|"
	done
}

printf "\n"

while read line; do
	    
	WORD1=$(echo "$line" | awk '{print $1}' )
	WORD2=$(echo "$line" | awk '{print $2}' )
	
	if [[ "$WORD1" == "WORD_SIZE_WRONG" ]]; then
	
		play_data "$WORD2"
	
	else
	
		play_data "$WORD1"
		
	fi

done <input


printf "\n"

#rm input
