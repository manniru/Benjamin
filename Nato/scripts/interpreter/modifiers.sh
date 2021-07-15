#!/bin/bash

#Key Code from input-event-codes.h

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "control" ]]; then

    OUTPUT="29"

elif [[ "$WORD" == "alt" ]]; then
    
    OUTPUT="56"

elif [[ "$WORD" == "shift" ]]; then
    
    OUTPUT="42"

fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.modifier string:"$OUTPUT"
    
fi
