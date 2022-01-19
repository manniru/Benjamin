#!/bin/bash
#Direction Keys
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

elif [[ "$WORD" == "raise" ]]; then
    
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

elif [[ "$WORD" == "tab" ]]; then
    
    OUTPUT="15"

elif [[ "$WORD" == "delete" ]]; then
    
    OUTPUT="111"

elif [[ "$WORD" == "slash" ]]; then
    
    OUTPUT="53"

elif [[ "$WORD" == "departure" ]]; then
    
    OUTPUT="1"

fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.debug string:"$WORD"
    dbus-send --session $DBUS_PATH.dirs  string:"$OUTPUT"
    
fi
