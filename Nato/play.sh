#!/bin/bash

CAT_NAME="scarlet"
AUDIO_PATH="audio/train/$CAT_NAME"
VLC_OPT="-q --gain 5 --play-and-exit --no-repeat --no-loop"

function play_data()
{
	FILEBASE=$1

	# \e[A: one line up - \e[K: clear line
	printf "\e[A\e[K\r$2     \n"
	printf "\e[K\r"
	sleep 0.5
	cvlc $VLC_OPT "$AUDIO_PATH/$FILEBASE.wav" 2>/dev/null & 

	for i in {1..60}; do
		read -rs -N1 -t0.1 REP </dev/tty
		
		if [[ "$REP" ]]; then
		
			printf "\e[K\rPress Any Key To Remove $FILEBASE, press q to abort"
			read -rs -N1 REP </dev/tty
			if [[ "$REP" != "q" ]]; then
				printf "\e[A\e[K\rRemoved: $2\n\n"
				rm "$AUDIO_PATH/$FILEBASE.wav"
			fi
			break
		fi
		
		printf "|"
	done
}

printf "\n"

while read line; do
	    
	F_BASE=$(echo "$line" | awk '{print $1}' )

	# if file exist
	if [[ -e "$AUDIO_PATH/$F_BASE.wav" ]]; then
		play_data "$F_BASE" "$line"
	fi

done <input

printf "\n"

#rm input
