#!/bin/bash

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "sim" ]]; then

    OUTPUT="${OUTPUT} "

elif [[ "$WORD" == "llllllll" ]]; then
    
    OUTPUT="${OUTPUT}b"

fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.speex string:"$OUTPUT"
    
fi
