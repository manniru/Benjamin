#!/bin/bash

#Key Code from input-event-codes.h

WORD="$1"
DBUS_PATH="$2"
OUTPUT=""

if [[ "$WORD" == "arch" ]]; then

    OUTPUT="a"

elif [[ "$WORD" == "bravo" ]]; then
    
    OUTPUT="b"

elif [[ "$WORD" == "catalina" ]]; then
    
    OUTPUT="c"

elif [[ "$WORD" == "charlie" ]]; then
    
    OUTPUT="c"

elif [[ "$WORD" == "delta" ]]; then
    
    OUTPUT="d"

elif [[ "$WORD" == "echo" ]]; then
    
    OUTPUT="e"

elif [[ "$WORD" == "fish" ]]; then
    
    OUTPUT="f"

elif [[ "$WORD" == "golf" ]]; then
    
    OUTPUT="g"

elif [[ "$WORD" == "hotel" ]]; then
    
    OUTPUT="h"
    
elif [[ "$WORD" == "india" ]]; then
    
    OUTPUT="i"
    
elif [[ "$WORD" == "jordan" ]]; then
    
    OUTPUT="j"
    
elif [[ "$WORD" == "kilo" ]]; then
    
    OUTPUT="k"
    
elif [[ "$WORD" == "limo" ]]; then
    
    OUTPUT="l"
    
elif [[ "$WORD" == "mike" ]]; then
    
    OUTPUT="m"
   
elif [[ "$WORD" == "november" ]]; then
    
    OUTPUT="n"

elif [[ "$WORD" == "oscar" ]]; then
    
    OUTPUT="o"

elif [[ "$WORD" == "papa" ]]; then
    
    OUTPUT="p"

elif [[ "$WORD" == "quebec" ]]; then
    
    OUTPUT="q"

elif [[ "$WORD" == "romeo" ]]; then
    
    OUTPUT="r"

elif [[ "$WORD" == "sierra" ]]; then
    
    OUTPUT="s"

elif [[ "$WORD" == "tango" ]]; then
    
    OUTPUT="t"

elif [[ "$WORD" == "uniform" ]]; then
    
    OUTPUT="u"

elif [[ "$WORD" == "u" ]]; then
    
    OUTPUT="u"

elif [[ "$WORD" == "eggs" ]]; then
    
    OUTPUT="x"

elif [[ "$WORD" == "yankee" ]]; then
    
    OUTPUT="y"

elif [[ "$WORD" == "zed" ]]; then
    
    OUTPUT="z"

fi

if [[ "$OUTPUT" ]]; then

    dbus-send --session $DBUS_PATH.nato string:"$OUTPUT"
    
fi
