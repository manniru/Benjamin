#!/bin/bash

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "open" ]]; then

    OUTPUT="1"

elif [[ "$WORD" == "system" ]]; then
    
    OUTPUT="2"

elif [[ "$WORD" == "start" ]]; then
    
    OUTPUT="3"

elif [[ "$WORD" == "fox" ]]; then
    
    OUTPUT="4"

elif [[ "$WORD" == "page" ]]; then
    
    OUTPUT="5"

elif [[ "$WORD" == "go" ]]; then
    
    OUTPUT="6"

elif [[ "$WORD" == "sky" ]]; then
    
    OUTPUT="7"

elif [[ "$WORD" == "dive" ]]; then
    
    OUTPUT="8"

elif [[ "$WORD" == "music" ]]; then
    
    OUTPUT="9"
    
elif [[ "$WORD" == "close" ]]; then
    
    OUTPUT="10"
    
elif [[ "$WORD" == "switch" ]]; then
    
    OUTPUT="11"
    
fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.debug string:"$WORD"
    dbus-send --session $DBUS_PATH.meta string:"$OUTPUT"
    
fi
