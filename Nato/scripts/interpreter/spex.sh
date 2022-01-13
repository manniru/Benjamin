#!/bin/bash
#Special Command
#Key Code from input-event-codes.h

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "special" ]]; then

    OUTPUT="57"

fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.spex  string:"$OUTPUT"
    
fi
