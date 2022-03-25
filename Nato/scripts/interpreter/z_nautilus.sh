#!/bin/bash

WORD="$1"
OUTPUT=""

if [[ "$WORD" == "fox" ]]; then

	OUTPUT="alt"

else

	OUTPUT="$WORD"

fi

echo $OUTPUT
