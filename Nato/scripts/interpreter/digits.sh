#!/bin/bash

#Key Code from input-event-codes.h

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "zero" ]]; then

    OUTPUT="11"

elif [[ "$WORD" == "one" ]]; then
    
    OUTPUT="2"

elif [[ "$WORD" == "two" ]]; then
    
    OUTPUT="3"

elif [[ "$WORD" == "three" ]]; then
    
    OUTPUT="4"

elif [[ "$WORD" == "four" ]]; then
    
    OUTPUT="5"

elif [[ "$WORD" == "five" ]]; then
    
    OUTPUT="6"

elif [[ "$WORD" == "six" ]]; then
    
    OUTPUT="7"

elif [[ "$WORD" == "seven" ]]; then
    
    OUTPUT="8"

elif [[ "$WORD" == "eight" ]]; then
    
    OUTPUT="9"
    
elif [[ "$WORD" == "nine" ]]; then
    
    OUTPUT="10"

fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.digit string:"$OUTPUT"
    
fi
