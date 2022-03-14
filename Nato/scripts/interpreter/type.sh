#!/bin/bash
#Reset Last Command (To Be Not Repeatable)

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "type" ]]; then
    
    OUTPUT="101"

fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.debug string:"$WORD"
    dbus-send --session $DBUS_PATH.type  string:"$OUTPUT"

fi
