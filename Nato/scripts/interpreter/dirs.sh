#!/bin/bash
#Direction Keys
#Number after Dir would repeat keys
#Key Code from input-event-codes.h

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "back" ]]; then
    
    OUTPUT="14"

elif [[ "$WORD" == "left" ]]; then
    
    OUTPUT="105"

elif [[ "$WORD" == "down" ]]; then
    
    OUTPUT="108"

elif [[ "$WORD" == "raise" ]]; then
    
    OUTPUT="103"

elif [[ "$WORD" == "right" ]]; then
    
    OUTPUT="106"

elif [[ "$WORD" == "slap" ]]; then
    
    OUTPUT="28"

elif [[ "$WORD" == "tab" ]]; then
    
    OUTPUT="15"

elif [[ "$WORD" == "delete" ]]; then
    
    OUTPUT="111"

elif [[ "$WORD" == "departure" ]]; then
    
    OUTPUT="1"

fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.debug string:"$WORD"
    dbus-send --session $DBUS_PATH.dirs  string:"$OUTPUT"
    
fi
