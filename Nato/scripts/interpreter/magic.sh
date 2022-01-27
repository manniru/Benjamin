#!/bin/bash

BUF="$1"
OUTPUT=""
ACTIVE_WIN_ID=$(xdotool getwindowfocus);
ACTIVE_WIN_TITLE=$(xdotool getwindowname $ACTIVE_WIN_ID)
ACTIVE_WIN_PID=$(xdotool getwindowpid $ACTIVE_WIN_ID)
ACTIVE_WIN_PROCESS=$(cat "/proc/$ACTIVE_WIN_PID/comm")
SI="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

for WORD in $BUF; do

	if [[ "$ACTIVE_WIN_TITLE" == "Open Files" ]] ||
	   [[ "$ACTIVE_WIN_PROCESS" == "nautilus" ]]; then
		
		KEYWORD=$($SI/z_nautilus.sh $WORD)
	    OUTPUT="$OUTPUT $KEYWORD"
	 
	else
	
	    OUTPUT="$OUTPUT $WORD"
	
	fi

done

echo $OUTPUT
