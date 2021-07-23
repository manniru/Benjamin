#!/bin/bash

#Key Code from input-event-codes.h

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "arch" ]]; then

    OUTPUT="30"

elif [[ "$WORD" == "bravo" ]]; then
    
    OUTPUT="48"

elif [[ "$WORD" == "catalina" ]]; then
    
    OUTPUT="46"

elif [[ "$WORD" == "charlie" ]]; then
    
    OUTPUT="46"

elif [[ "$WORD" == "delta" ]]; then
    
    OUTPUT="32"

elif [[ "$WORD" == "echo" ]]; then
    
    OUTPUT="18"

elif [[ "$WORD" == "fish" ]]; then
    
    OUTPUT="33"

elif [[ "$WORD" == "golf" ]]; then
    
    OUTPUT="34"

elif [[ "$WORD" == "hotel" ]]; then
    
    OUTPUT="35"
    
elif [[ "$WORD" == "india" ]]; then
    
    OUTPUT="23"
    
elif [[ "$WORD" == "jordan" ]]; then
    
    OUTPUT="36"
    
elif [[ "$WORD" == "kilo" ]]; then
    
    OUTPUT="37"
    
elif [[ "$WORD" == "limo" ]]; then
    
    OUTPUT="38"
    
elif [[ "$WORD" == "mike" ]]; then
    
    OUTPUT="50"
   
elif [[ "$WORD" == "november" ]]; then
    
    OUTPUT="49"

elif [[ "$WORD" == "oscar" ]]; then
    
    OUTPUT="24"

elif [[ "$WORD" == "papa" ]]; then
    
    OUTPUT="25"

elif [[ "$WORD" == "quebec" ]]; then
    
    OUTPUT="16"

elif [[ "$WORD" == "romeo" ]]; then
    
    OUTPUT="19"

elif [[ "$WORD" == "sierra" ]]; then
    
    OUTPUT="31"

elif [[ "$WORD" == "tango" ]]; then
    
    OUTPUT="20"

elif [[ "$WORD" == "uniform" ]]; then
    
    OUTPUT="22"

elif [[ "$WORD" == "u" ]]; then
    
    OUTPUT="22"

elif [[ "$WORD" == "eggs" ]]; then
    
    OUTPUT="45"

elif [[ "$WORD" == "vpn" ]]; then
    
    OUTPUT="47"

elif [[ "$WORD" == "wake" ]]; then
    
    OUTPUT="17"

elif [[ "$WORD" == "yankee" ]]; then
    
    OUTPUT="21"

elif [[ "$WORD" == "zed" ]]; then
    
    OUTPUT="44"

fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.nato string:"$OUTPUT"
    
fi
