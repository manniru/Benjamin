#!/bin/bash

#Remove spuroius prefix (first word)
BUF=$( echo "$1" | cut -d " " -f2-)
OUTPUT=""
SI="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

DBUS_PATH="--dest=com.binaee.rebound / com.binaee.rebound"

for WORD in $BUF; do

    $SI/digits.sh $WORD "$DBUS_PATH"
    $SI/nato.sh $WORD "$DBUS_PATH"
    $SI/meta.sh $WORD "$DBUS_PATH"
    $SI/commands.sh $WORD "$DBUS_PATH"
    $SI/modifiers.sh $WORD "$DBUS_PATH"
    
done

echo $BUF
dbus-send --session $DBUS_PATH.exec

