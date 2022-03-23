#!/bin/bash

WORD="$1"
OUTPUT=""

if [[ "$WORD" == "go" ]]; then
		
	OUTPUT="alt raise"
				
elif [[ "$WORD" == "fox" ]]; then

	OUTPUT="alt"

else

	OUTPUT="$WORD"

fi

echo $OUTPUT
