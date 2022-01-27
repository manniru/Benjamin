#!/bin/bash

WORD="$1"
OUTPUT=""

if [[ "$WORD" == "go" ]]; then
		
	OUTPUT="special alt raise"
		
elif [[ "$WORD" == "sky" ]]; then

	OUTPUT="alt raise"
		
elif [[ "$WORD" == "fox" ]]; then

	OUTPUT="alt"

else

	OUTPUT="$WORD"

fi

echo $OUTPUT
