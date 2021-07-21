#!/bin/bash

#Key Code from input-event-codes.h

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "sim" ]]; then

    OUTPUT="57"

elif [[ "$WORD" == "back" ]]; then
    
    OUTPUT="14"

elif [[ "$WORD" == "left" ]]; then
    
    OUTPUT="105"

elif [[ "$WORD" == "down" ]]; then
    
    OUTPUT="108"

elif [[ "$WORD" == "up" ]]; then
    
    OUTPUT="103"

elif [[ "$WORD" == "right" ]]; then
    
    OUTPUT="106"

elif [[ "$WORD" == "end" ]]; then
    
    OUTPUT="107"

elif [[ "$WORD" == "home" ]]; then
    
    OUTPUT="102"

elif [[ "$WORD" == "semi" ]]; then
    
    OUTPUT="39"

elif [[ "$WORD" == "slap" ]]; then
    
    OUTPUT="28"

elif [[ "$WORD" == "period" ]]; then
    
    OUTPUT="52"

elif [[ "$WORD" == "comma" ]]; then
    
    OUTPUT="51"

elif [[ "$WORD" == "dash" ]]; then
    
    OUTPUT="12"
    
elif [[ "$WORD" == "equal" ]]; then
    
    OUTPUT="13"

elif [[ "$WORD" == "plus" ]]; then
    
    OUTPUT="78"

elif [[ "$WORD" == "super" ]]; then
    
    OUTPUT="125"

elif [[ "$WORD" == "tab" ]]; then
    
    OUTPUT="15"

fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.speex string:"$OUTPUT"
    
fi
