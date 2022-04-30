#!/bin/bash

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "firefox" ]]; then
    
    OUTPUT="1"

elif [[ "$WORD" == "files" ]]; then
    
    OUTPUT="2"

elif [[ "$WORD" == "spotify" ]]; then

    OUTPUT="3"

elif [[ "$WORD" == "atom" ]]; then

    OUTPUT="4"

elif [[ "$WORD" == "ding" ]]; then

    OUTPUT="5"

elif [[ "$WORD" == "link" ]]; then

    OUTPUT="6"

elif [[ "$WORD" == "sleep" ]]; then

    OUTPUT="7"

fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.debug string:"$WORD"
    dbus-send --session $DBUS_PATH.apps  string:"$OUTPUT"

fi