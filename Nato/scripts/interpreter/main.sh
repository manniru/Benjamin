#!/bin/bash

BUF="$1"
OUTPUT=""
SI="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

DBUS_PATH="--dest=com.binaee.rebound / com.binaee.rebound"

for WORD in $BUF; do

    $SI/nato.sh $WORD "$DBUS_PATH"
    $SI/meta.sh $WORD "$DBUS_PATH"
    $SI/apps.sh $WORD "$DBUS_PATH"
    $SI/dirs.sh $WORD "$DBUS_PATH"
    $SI/digits.sh $WORD "$DBUS_PATH"
    $SI/modifiers.sh $WORD "$DBUS_PATH"
    
done
