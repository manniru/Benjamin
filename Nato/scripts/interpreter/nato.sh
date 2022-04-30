#!/bin/bash
#Keyboard Alphabets
#Number after NATO would not repeat
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

elif [[ "$WORD" == "sim" ]]; then

    OUTPUT="57"

elif [[ "$WORD" == "end" ]]; then
    
    OUTPUT="107"

elif [[ "$WORD" == "home" ]]; then
    
    OUTPUT="102"

elif [[ "$WORD" == "semi" ]]; then
    
    OUTPUT="39"

elif [[ "$WORD" == "period" ]]; then
    
    OUTPUT="52"

elif [[ "$WORD" == "comma" ]]; then
    
    OUTPUT="51"

elif [[ "$WORD" == "dash" ]]; then
    
    OUTPUT="12"
    
elif [[ "$WORD" == "equal" ]]; then
    
    OUTPUT="13"

elif [[ "$WORD" == "plus" ]]; then
    
    OUTPUT="78"

elif [[ "$WORD" == "slash" ]]; then
    
    OUTPUT="53"

elif [[ "$WORD" == "underline" ]]; then
    
    OUTPUT="53"
#    xdotool key shift+minus make problem when sleep

elif [[ "$WORD" == "bracket" ]]; then
    
    OUTPUT="26"

elif [[ "$WORD" == "quote" ]]; then
    
    OUTPUT="40"
    
elif [[ "$WORD" == "github" ]]; then
    
    OUTPUT="58" #caps lock

fi


if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.debug string:"$WORD"
    dbus-send --session $DBUS_PATH.nato string:"$OUTPUT"
    
fi
