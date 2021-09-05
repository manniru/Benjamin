#!/bin/bash

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "github" ]]; then

    OUTPUT="1"

elif [[ "$WORD" == "firefox" ]]; then
    
    OUTPUT="2"

elif [[ "$WORD" == "files" ]]; then
    
    OUTPUT="3"

elif [[ "$WORD" == "spotify" ]]; then

    OUTPUT="4"

elif [[ "$WORD" == "atom" ]]; then

    OUTPUT="5"

elif [[ "$WORD" == "ding" ]]; then

    OUTPUT="6"

fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.debug string:"$WORD"
    dbus-send --session $DBUS_PATH.apps  string:"$OUTPUT"

fi
