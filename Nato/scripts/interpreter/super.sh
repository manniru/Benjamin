#!/bin/bash
#Meta Commands in Super Mode

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "meta" ]]; then
    
    OUTPUT="101"
    
elif [[ "$WORD" == "colon" ]]; then
    
    OUTPUT="102"
    
elif [[ "$WORD" == "switch" ]]; then
    
    OUTPUT="103"
    
elif [[ "$WORD" == "kick" ]]; then
    
    OUTPUT="104"
    
fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.debug string:"$WORD"
    dbus-send --session $DBUS_PATH.super string:"$OUTPUT"

fi
