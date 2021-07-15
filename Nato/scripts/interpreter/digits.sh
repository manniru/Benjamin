#!/bin/bash

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "zero" ]]; then

    OUTPUT="0"

elif [[ "$WORD" == "one" ]]; then
    
    OUTPUT="1"

elif [[ "$WORD" == "two" ]]; then
    
    OUTPUT="2"

elif [[ "$WORD" == "three" ]]; then
    
    OUTPUT="3"

elif [[ "$WORD" == "four" ]]; then
    
    OUTPUT="4"

elif [[ "$WORD" == "five" ]]; then
    
    OUTPUT="5"

elif [[ "$WORD" == "six" ]]; then
    
    OUTPUT="6"

elif [[ "$WORD" == "seven" ]]; then
    
    OUTPUT="7"

elif [[ "$WORD" == "eight" ]]; then
    
    OUTPUT="8"
    
elif [[ "$WORD" == "nine" ]]; then
    
    OUTPUT="9"

fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.digit string:"$OUTPUT"
    
fi
